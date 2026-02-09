#include <span>
#include <algorithm>
#include <boost/unordered/unordered_flat_set.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include "XmlBin.hpp"
#include "Input.hpp"
#include "Output.hpp"
#include "Sort.hpp"

using namespace xmlbin;

struct XmlContext : Context {
    template<class T>
    const T* data(ByteOffset offset) const {
        return reinterpret_cast<const T*>(reinterpret_cast<const char*>(this) +
                                          offset);
    }

    std::string_view get(StrId idx) const {
        return data<char>(strings.offset) + idx.value;
    }

    std::string_view getOr(StrId idx, std::string_view other) const {
        return idx ? get(idx) : other;
    }

    std::span<const StrId> getUniqueStrList() const {
        return {data<StrId>(uniqueStrings.offset) + 1, uniqueStrings.count - 1};
    }

    NodeId get(Idx<NodeId> idx) const {
        return data<NodeId>(nodes.offset)[idx.value];
    }

    std::span<const NodeId> getList(Sequence<NodeId> seq) const {
        return {data<NodeId>(nodes.offset) + seq.start.value, seq.count};
    }

    const Attribute& get(Idx<Attribute> idx) const {
        return data<Attribute>(attrs.offset)[idx.value];
    }

    std::span<const Attribute> getList(Sequence<Attribute> seq) const {
        return {data<Attribute>(attrs.offset) + seq.start.value, seq.count};
    }

    const Element& get(Idx<Element> idx) const {
        return data<Element>(elems.offset)[idx.value];
    }

    StrId getUniqueStr(std::string_view str) const {
        Index k = 1;
        const auto t = data<StrId>(uniqueStrings.offset);
        while (k <= uniqueStrings.count) {
            const auto idx = t[k];
            const auto cmp = get(idx) <=> str;
            if (cmp == std::strong_ordering::equal)
                return idx;
            k = (k << 1u) | Index(cmp == std::strong_ordering::less);
        }
        return {};
    }
};

StrId findAttr(std::span<const Attribute> attrList, StrId nameId) {
    Index k = 1;
    while (k <= attrList.size()) {
        const auto& attr = attrList[k - 1];
        if (attr.name == nameId)
            return attr.value;
        k = (k << 1u) | Index(attr.name < nameId);
    }
    return {};
}

std::string_view trimR(std::string_view str) {
    auto p = str.end();
    const auto b = str.begin();
    while (p != b) {
        --p;
        if (*p != ' ') {
            ++p;
            break;
        }
    }
    return {b, p};
}

bool consumeMatch(std::string_view& str, std::string_view match) {
    if (str.starts_with(match)) {
        str.remove_prefix(match.size());
        return true;
    }
    return false;
}

struct Builder {
    enum class TypeKind { Raw, Enum, Bitmask, Alias, Struct, Handle, NUM };

    using TypeId = TagId<TypeKind>;

    struct NameMatch {
        std::string_view m_str;
        std::string_view m_num;
        std::string_view m_ext;
    };

    struct TypeInfo {
        std::string_view m_name;
        const Element* m_elem;
    };

    struct DefInfo {
        std::string_view m_name;
        std::string_view m_def;
    };

    struct BitmaskInfo {
        std::string_view m_name;
        std::string_view m_type;
        std::string_view m_enum;
    };

    enum VarTag : std::uint8_t { Normal, Slave, Master };

    struct VarInfo {
        std::string_view m_typePrefix;
        std::string_view m_type;
        std::string_view m_typeSuffix;
        std::string_view m_name;
        std::string_view m_array;
        std::string_view m_comment;
    };

    struct MemberInfo : VarInfo {
        std::string m_newType;
        const char* m_slaveName = nullptr;
        StrId m_valuesAttr;
        bool m_optional = false;
        bool m_addCast = false;
        bool m_isPtr = false;
        bool m_isArr = false;
        bool m_isStr = false;
        bool m_isStruct = false;
        std::uint8_t m_tag = Normal;
    };

    struct EnumExtendInfo {
        const Element& m_elem;
        StrId m_guard;
    };

    struct CommandInfo {
        StrId m_name;
        const Element& m_elem;
    };

    struct CommandTypeInfo {
        StrId m_type;
        const Element& m_elem;
    };

    const XmlContext& m_ctx;
    boost::unordered_flat_set<std::pair<std::string_view, std::string_view>>
        m_typeDeps;
    std::vector<TypeId> m_typeIds;
    boost::unordered_flat_set<std::string_view> m_exts;
    boost::unordered_flat_set<std::string_view> m_raws;
    boost::unordered_flat_set<std::string_view> m_structs;
    boost::unordered_flat_set<std::string_view> m_enumOrFlag;
    boost::unordered_flat_set<std::string_view> m_scopes;
    boost::unordered_flat_map<std::string_view, StrId> m_supported;
    boost::unordered_flat_map<std::string_view, std::vector<CommandInfo>>
        m_handleCommands;
    boost::unordered_flat_map<std::string_view, std::vector<const Element*>>
        m_structExtendsMap;
    boost::unordered_flat_map<std::string_view, std::vector<EnumExtendInfo>>
        m_enumExtendsMap;
    boost::unordered_flat_map<std::string_view, CommandTypeInfo>
        m_commandElemMap;
    boost::unordered_flat_map<std::string_view, const Element*>
        m_internalFeatureMap;
    std::vector<CommandInfo> m_globalCommands;
    std::vector<TypeInfo> m_typeInfos;
    std::vector<DefInfo> m_defInfo;
    std::vector<BitmaskInfo> m_bitmaskInfo;

    const StrId tagsTag = m_ctx.getUniqueStr("tags");
    const StrId tagTag = m_ctx.getUniqueStr("tag");
    const StrId enumsTag = m_ctx.getUniqueStr("enums");
    const StrId enumTag = m_ctx.getUniqueStr("enum");
    const StrId typesTag = m_ctx.getUniqueStr("types");
    const StrId typeTag = m_ctx.getUniqueStr("type");
    const StrId categoryTag = m_ctx.getUniqueStr("category");
    const StrId nameTag = m_ctx.getUniqueStr("name");
    const StrId aliasTag = m_ctx.getUniqueStr("alias");
    const StrId memberTag = m_ctx.getUniqueStr("member");
    const StrId commentTag = m_ctx.getUniqueStr("comment");
    const StrId bitvaluesTag = m_ctx.getUniqueStr("bitvalues");
    const StrId requiresTag = m_ctx.getUniqueStr("requires");
    const StrId requireTag = m_ctx.getUniqueStr("require");
    const StrId deprecatedTag = m_ctx.getUniqueStr("deprecated");
    const StrId deprecatedbyTag = m_ctx.getUniqueStr("deprecatedby");
    const StrId bitwidthTag = m_ctx.getUniqueStr("bitwidth");
    const StrId returnedonlyTag = m_ctx.getUniqueStr("returnedonly");
    const StrId extensionsTag = m_ctx.getUniqueStr("extensions");
    const StrId extensionTag = m_ctx.getUniqueStr("extension");
    const StrId numberTag = m_ctx.getUniqueStr("number");
    const StrId extendsTag = m_ctx.getUniqueStr("extends");
    const StrId featureTag = m_ctx.getUniqueStr("feature");
    const StrId structextendsTag = m_ctx.getUniqueStr("structextends");
    const StrId dependsTag = m_ctx.getUniqueStr("depends");
    const StrId valuesTag = m_ctx.getUniqueStr("values");
    const StrId optionalTag = m_ctx.getUniqueStr("optional");
    const StrId parentTag = m_ctx.getUniqueStr("parent");
    const StrId commandsTag = m_ctx.getUniqueStr("commands");
    const StrId commandTag = m_ctx.getUniqueStr("command");
    const StrId protoTag = m_ctx.getUniqueStr("proto");
    const StrId paramTag = m_ctx.getUniqueStr("param");
    const StrId lenTag = m_ctx.getUniqueStr("len");
    const StrId apiTag = m_ctx.getUniqueStr("api");
    const StrId apitypeTag = m_ctx.getUniqueStr("apitype");
    const StrId supportedTag = m_ctx.getUniqueStr("supported");
    const StrId platformTag = m_ctx.getUniqueStr("platform");
    const StrId protectTag = m_ctx.getUniqueStr("protect");
    const StrId removeTag = m_ctx.getUniqueStr("remove");
    const StrId objtypeenumTag = m_ctx.getUniqueStr("objtypeenum");
    const StrId provisionalTag = m_ctx.getUniqueStr("provisional");

    struct GenState {
        bool m_delim = false;
        StrId m_guard;
    };

    const StrId* findSupport(std::string_view name) const {
        const auto it = m_supported.find(name);
        if (it == m_supported.end())
            return nullptr;
        return &it->second;
    }

    static StrId subGuard(StrId baseGuard, StrId guard) {
        return guard > baseGuard ? guard : StrId{};
    }

    StrId updateGuard(Output& os, StrId guard, GenState& state) {
        if (guard == state.m_guard)
            return {};
        if (state.m_guard)
            os << "#endif // " << m_ctx.get(state.m_guard) << '\n';
        state.m_guard = guard;
        return guard;
    }

    void generateGuard(Output& os, StrId guard) {
        if (guard)
            os << "#if " << m_ctx.get(guard) << '\n';
    }

    void generate(Output& os) {
        os << "#ifndef VKLITE_VULKAN_HPP\n"
              "#define VKLITE_VULKAN_HPP\n"
              "\n"
              "#include \"core.hpp\"\n"
              "\n"
              "namespace vklite {\n";
        GenState state;
        auto lastKind = TypeKind::Raw;
        for (const auto typeId : m_typeIds) {
            const auto kind = typeId.getKind();
            if (lastKind != kind) {
                lastKind = kind;
                state.m_delim = true;
            }
            switch (kind) {
            case TypeKind::Raw: generateRaw(os, typeId, state); break;
            case TypeKind::Enum: generateEnum(os, typeId, state); break;
            case TypeKind::Bitmask: generateBitmask(os, typeId, state); break;
            case TypeKind::Alias: generateAlias(os, typeId, state); break;
            case TypeKind::Struct: generateStruct(os, typeId, state); break;
            case TypeKind::Handle: generateHandle(os, typeId, state); break;
            default: break;
            }
        }
        state.m_delim = true;
        for (const auto& cmd : m_globalCommands) {
            generateCommand(os, cmd, {}, {}, state);
        }
        updateGuard(os, {}, state);
        os << "}\n"
              "\n"
              "#endif // VKLITE_VULKAN_HPP";
    }

    static bool isCapital(char c) { return 'A' <= c && c <= 'Z'; }

    static bool isDigit(char c) { return '0' <= c && c <= '9'; }

    static char toCapital(char c) {
        return 'a' <= c && c <= 'z' ? c + ('A' - 'a') : c;
    }

    static char toLower(char c) {
        return 'A' <= c && c <= 'Z' ? c + ('a' - 'A') : c;
    }

    template<class It>
    static bool AppendCase(std::string& out, It i, It e, int caseDelta) {
        if (i == e)
            return false;
        const auto offset = out.size();
        out.resize(offset + (e - i));
        auto p = out.data() + offset;
        do {
            *p++ = *i + caseDelta;
        } while (++i != e);
        return true;
    }

    static void camelToCapital(std::string_view str, std::string& out) {
        constexpr int upper = 'A' - 'a';
        auto b = str.begin();
        auto p = b;
        const auto e = str.end();
        while (p != e) {
            const auto c = *p;
            if (isCapital(c)) {
                if (AppendCase(out, b, p, upper))
                    out += '_';
                out += c;
                b = ++p;
            } else if (isDigit(c)) {
                if (AppendCase(out, b, p, upper))
                    out += '_';
                b = p;
                while (++p != e && isDigit(*p)) {}
                out.append(b, p);
                b = p;
            } else {
                ++p;
            }
        }
        AppendCase(out, b, p, upper);
    }

    static void capitalToCamel(std::string_view str, std::string& out) {
        constexpr int lower = 'a' - 'A';
        auto p = str.begin();
        const auto e = str.end();
        while (p != e) {
            const auto c = *p;
            if (c == '_') {
                ++p;
            } else if (isCapital(c)) {
                out += c;
                const auto b = ++p;
                while (p != e && isCapital(*p)) {
                    ++p;
                }
                AppendCase(out, b, p, lower);
            } else if (isDigit(c)) {
                const auto b = p;
                while (++p != e && isDigit(*p)) {}
                out.append(b, p);
            } else {
                out += c;
                ++p;
            }
        }
    }

    bool matchName(std::string_view str, NameMatch& match) const {
        const auto b = str.begin();
        auto e = str.end();
        auto p = e;
        while (p != b) {
            --p;
            if (!isCapital(*p)) {
                match.m_ext = {++p, e};
                e = p;
                break;
            }
        }
        while (p != b) {
            --p;
            if (!isDigit(*p)) {
                match.m_num = {++p, e};
                e = p;
                break;
            }
        }
        if (p != b) {
            match.m_str = {b, p};
            return true;
        }
        return false;
    }

    std::string_view getExt(std::string_view str) const {
        const auto p = str.find_last_of('_');
        if (p != std::string_view::npos) {
            str.remove_prefix(p + 1);
            if (m_exts.contains(str))
                return str;
        }
        return {};
    }

    std::span<const EnumExtendInfo>
    findEnumExtends(std::string_view name) const {
        const auto it = m_enumExtendsMap.find(name);
        if (it != m_enumExtendsMap.end())
            return it->second;
        return {};
    }

    std::span<const Element* const>
    findStructExtends(std::string_view name) const {
        const auto it = m_structExtendsMap.find(name);
        if (it != m_structExtendsMap.end())
            return it->second;
        return {};
    }

    std::span<const CommandInfo> findCommands(std::string_view name) const {
        const auto it = m_handleCommands.find(name);
        if (it != m_handleCommands.end())
            return it->second;
        return {};
    }

    void addRaw(std::string_view name) {
        m_typeIds.push_back({TypeKind::Raw, Index(m_typeInfos.size())});
        m_typeInfos.push_back({name});
        m_raws.insert(name);
    }

    void generateRaw(Output& os, TypeId typeId, GenState& state) {
        const auto& typeInfo = m_typeInfos[typeId.getIndex()];
        const auto support = findSupport(typeInfo.m_name);
        if (!support)
            return;
        const auto guard = updateGuard(os, *support, state);
        if (state.m_delim) {
            os << '\n';
            state.m_delim = false;
        }
        generateGuard(os, guard);
        os << "using " << typeInfo.m_name << " = Vk" << typeInfo.m_name
           << ";\n";
    }

    void generateBitmask(Output& os, TypeId typeId, GenState& state) {
        const auto& bitmaskInfo = m_bitmaskInfo[typeId.getIndex()];
        const auto support = findSupport(bitmaskInfo.m_name);
        if (!support)
            return;
        if (bitmaskInfo.m_enum.empty() ||
            !m_supported.contains(bitmaskInfo.m_enum)) {
            const auto guard = updateGuard(os, *support, state);
            if (state.m_delim) {
                os << '\n';
                state.m_delim = false;
            }
            generateGuard(os, guard);
            os << "using " << bitmaskInfo.m_name << " = " << bitmaskInfo.m_type
               << ";\n";
        } else {
            const auto guard = updateGuard(os, *support, state);
            os << '\n';
            state.m_delim = true;
            generateGuard(os, guard);
            os << "using " << bitmaskInfo.m_name << " = FlagSet<"
               << bitmaskInfo.m_enum << ", " << bitmaskInfo.m_type << ">;\n";
            os << "constexpr " << bitmaskInfo.m_name << " operator|("
               << bitmaskInfo.m_enum << " a, " << bitmaskInfo.m_enum
               << " b) noexcept { return " << bitmaskInfo.m_name << '('
               << bitmaskInfo.m_type << "(a) | " << bitmaskInfo.m_type
               << "(b)); }\n";
        }
    }

    void generateAlias(Output& os, TypeId typeId, GenState& state) {
        const auto& defInfo = m_defInfo[typeId.getIndex()];
        const auto support = findSupport(defInfo.m_name);
        if (!support)
            return;
        const auto guard = updateGuard(os, *support, state);
        if (state.m_delim) {
            os << '\n';
            state.m_delim = false;
        }
        generateGuard(os, guard);
        os << "using " << defInfo.m_name << " = " << defInfo.m_def << ";\n";
    }

    void generateEnum(Output& os, TypeId typeId, GenState& state) {
        const auto& typeInfo = m_typeInfos[typeId.getIndex()];
        const auto support = findSupport(typeInfo.m_name);
        if (!support)
            return;
        NameMatch match;
        if (!matchName(typeInfo.m_name, match)) {
            print("BAD NAME: {}\n", typeInfo.m_name);
            return;
        }
        const auto attrs = m_ctx.getList(typeInfo.m_elem->attrs);
        const bool isBitmask = m_ctx.get(findAttr(attrs, typeTag)) == "bitmask";
        if (isBitmask) {
            if (!match.m_str.ends_with("FlagBits")) {
                print("BAD BITMASK NAME: {}\n", match.m_str);
                return;
            }
            match.m_str.remove_suffix(8);
        }
        const auto guard = updateGuard(os, *support, state);
        os << '\n';
        state.m_delim = true;
        generateGuard(os, guard);
        if (const auto commentAttr = findAttr(attrs, commentTag)) {
            os << "// " << m_ctx.get(commentAttr) << '\n';
        }
        os << "enum class " << typeInfo.m_name << " : ";
        if (const auto bitwidthAttr = findAttr(attrs, bitwidthTag)) {
            os << "uint" << m_ctx.get(bitwidthAttr) << "_t";
        } else {
            if (isBitmask)
                os << 'u';
            os << "int32_t";
        }
        os << " {\n";
        std::string prefix("VK_");
        const bool isResult = typeInfo.m_name == "Result";
        if (!isResult) [[likely]] {
            camelToCapital(match.m_str, prefix);
            if (!match.m_num.empty()) {
                prefix += '_';
                prefix += match.m_num;
            }
            prefix += '_';
        }
        std::vector<std::pair<std::string, StrId>> resultEnums;
        boost::unordered_flat_set<std::string_view> uniqueIds;
        GenState stateEnum;
        const auto each = [&](const Element& elem, StrId extGuard = {}) {
            const auto attrs = m_ctx.getList(elem.attrs);
            if (findAttr(attrs, deprecatedTag))
                return;
            const auto name = m_ctx.get(findAttr(attrs, nameTag));
            auto sub = name;
            if (!consumeMatch(sub, prefix))
                return;
            if (extGuard && !uniqueIds.insert(sub).second)
                return;
            const auto ext = getExt(sub);
            if (!ext.empty())
                sub.remove_suffix(ext.size() + 1);
            std::string eName("e");
            if (isBitmask && sub.ends_with("_BIT")) {
                eName[0] = 'b';
                sub.remove_suffix(4);
            }
            capitalToCamel(sub, eName);
            if (match.m_ext != ext)
                eName += ext;
            const auto guardEnum = updateGuard(os, extGuard, stateEnum);
            generateGuard(os, guardEnum);
            if (const auto commentAttr = findAttr(attrs, commentTag)) {
                os << "  // " << m_ctx.get(commentAttr) << '\n';
            }
            os << "  " << eName << " = " << name << ",\n";
            if (isResult) [[unlikely]] {
                if (!findAttr(attrs, aliasTag))
                    resultEnums.emplace_back(std::move(eName), extGuard);
            }
        };
        processChildElems(*typeInfo.m_elem, enumTag, each);
        for (const auto enumExtend : findEnumExtends(typeInfo.m_name)) {
            each(enumExtend.m_elem, enumExtend.m_guard);
        }
        updateGuard(os, {}, stateEnum);
        os << "};\n";
        if (isResult) [[unlikely]] {
            os << "\ninline const char* getResultText(Result r) noexcept {\n"
                  "  using enum Result;\n"
                  "  switch (r) {\n";
            stateEnum = {};
            for (const auto& resultEnum : resultEnums) {
                const std::string_view name = resultEnum.first;
                const auto guardEnum =
                    updateGuard(os, resultEnum.second, stateEnum);
                generateGuard(os, guardEnum);
                os << "  case " << name << ": return \"" << name.substr(1)
                   << "\";\n";
            }
            updateGuard(os, {}, stateEnum);
            os << "  default: return \"\";\n"
                  "  }\n"
                  "}\n";
        }
    }

    bool checkApi(std::span<const Attribute> attrs) const {
        if (const auto apiAttr = findAttr(attrs, apiTag)) {
            if (!findCommaList(m_ctx.get(apiAttr), "vulkan"))
                return false;
        }
        return !findAttr(attrs, deprecatedTag);
    }

    void generateMemberName(Output& os, std::string_view name, bool isPtr) {
        if (isPtr) {
            const auto offset = name.find_first_not_of('p');
            if (offset != 0) {
                os << name.substr(offset);
                return;
            }
        }
        os << toCapital(name[0]);
        os << name.substr(1);
    }

    VarInfo getVarInfo(const Element& elem) const {
        VarInfo info;
        unsigned i = 0;
        for (const auto child : m_ctx.getList(elem.children)) {
            if (child.getKind() == NodeKind::Text) {
                const auto str = m_ctx.get(StrId{child.getIndex()});
                switch (i) {
                case 0: info.m_typePrefix = str; break;
                case 1: info.m_typeSuffix = trimR(str); break;
                case 2:
                    if (str.starts_with('[') && str.ends_with(']')) {
                        info.m_array = str;
                        info.m_array.remove_prefix(1);
                        info.m_array.remove_suffix(1);
                    }
                    break;
                }
                i = 3;
            } else { // Element
                const auto& elem = m_ctx.get(Idx<Element>{child.getIndex()});
                const auto str = getText(elem);
                if (elem.tag == typeTag) {
                    i = 1;
                    info.m_type = str;
                } else if (elem.tag == nameTag) {
                    i = 2;
                    info.m_name = str;
                } else if (elem.tag == enumTag) {
                    i = 3;
                    info.m_array = str;
                } else if (elem.tag == commentTag) {
                    i = 3;
                    info.m_comment = str;
                }
            }
        }
        return info;
    }

    MemberInfo getMemberInfo(const Element& elem) {
        const auto attrs = m_ctx.getList(elem.attrs);
        MemberInfo info{getVarInfo(elem)};
        info.m_optional = !!findAttr(attrs, optionalTag);
        info.m_valuesAttr = findAttr(attrs, valuesTag);
        info.m_addCast =
            consumeMatch(info.m_type, "Vk") && !m_raws.contains(info.m_type);
        info.m_isPtr = info.m_typeSuffix.ends_with('*');
        info.m_isArr = !info.m_array.empty();
        if (info.m_isArr && info.m_type == "char") {
            info.m_isStr = true;
            info.m_newType = "std::string_view";
        } else {
            if (info.m_isArr) {
                info.m_newType += "std::span<const ";
            } else {
                info.m_isStruct = info.m_typePrefix.empty() &&
                                  info.m_typeSuffix.empty() &&
                                  m_structs.contains(info.m_type);
            }
            if (info.m_isStruct) {
                info.m_newType += "const ";
                info.m_newType += info.m_type;
                info.m_newType += '&';
            } else {
                info.m_newType += info.m_typePrefix;
                info.m_newType += info.m_type;
                info.m_newType += info.m_typeSuffix;
            }
            if (info.m_isArr) {
                info.m_newType += ", ";
                info.m_newType += info.m_array;
                info.m_newType += '>';
            }
        }
        return info;
    }

    void generateMemberInit(Output& os, const MemberInfo& info,
                            std::string_view name,
                            const char* subName = nullptr) {
        const bool addCast = info.m_addCast && !info.m_isStruct;
        if (addCast) {
            os << "std::bit_cast<" << info.m_typePrefix << "Vk" << info.m_type
               << info.m_typeSuffix << ">(";
        }
        os << name;
        if (subName)
            os << subName;
        if (addCast)
            os << ')';
    }

    void generateMemberSetSlot(Output& os, const MemberInfo& info) {
        if (info.m_isArr) {
            if (info.m_isStr) {
                os << "const auto len = std::max<std::size_t>(" << info.m_array
                   << " - 1, value.size()); ";
            }
            os << "std::memcpy(&this->" << info.m_name << ", value.data(), ";
            if (info.m_isStr)
                os << "len); this->" << info.m_name << "[len] = '\\0'";
            else
                os << "value.size_bytes()" << ')';
        } else {
            os << "this->" << info.m_name << " = ";
            generateMemberInit(os, info, "value", info.m_slaveName);
        }
    }

    void generateMemberGetSlot(Output& os, const MemberInfo& info) {
        if (info.m_isArr) {
            if (info.m_addCast) {
                os << info.m_newType << "(std::bit_cast<const "
                   << info.m_typePrefix << info.m_type << info.m_typeSuffix
                   << "*>(&this->" << info.m_name << "), " << info.m_array
                   << ')';
            } else {
                os << "this->" << info.m_name;
            }
        } else {
            if (info.m_addCast) {
                if (info.m_isStruct) {
                    os << "static_cast<const " << info.m_typePrefix
                       << info.m_type << info.m_typeSuffix << "&>(";
                } else {
                    os << "std::bit_cast<" << info.m_typePrefix << info.m_type
                       << info.m_typeSuffix << ">(";
                }
            }
            os << "this->" << info.m_name;
            if (info.m_addCast)
                os << ')';
        }
    }

    void generateMember(Output& os, const MemberInfo& info, bool returnedonly) {
        if (info.m_tag == VarTag::Slave)
            return;
        if (!info.m_comment.empty())
            os << "  // " << info.m_comment << '\n';
        const bool isComposite = info.m_tag >= VarTag::Master;
        if (!returnedonly) {
            os << "  void set";
            generateMemberName(os, info.m_name, info.m_isPtr);
            os << '(' << info.m_newType << " value) { ";
            if (isComposite) {
                const auto slotCount = info.m_tag - VarTag::Master;
                const auto slot = &info - slotCount;
                for (unsigned i = 0; i != slotCount; ++i) {
                    if (i)
                        os << "; ";
                    generateMemberSetSlot(os, slot[i]);
                }
            } else {
                generateMemberSetSlot(os, info);
            }
            os << "; }\n";
        }
        os << "  " << info.m_newType << " get";
        generateMemberName(os, info.m_name, info.m_isPtr);
        os << "() const { return ";
        if (isComposite) {
            const auto slotCount = info.m_tag - VarTag::Master;
            const auto slot = &info - slotCount;
            os << '{';
            for (unsigned i = 0; i != slotCount; ++i) {
                if (i)
                    os << ", ";
                generateMemberGetSlot(os, slot[i]);
            }
            os << '}';
        } else {
            generateMemberGetSlot(os, info);
        }
        os << "; }\n";
    }

    template<class Info>
    static bool isAnyOptional(const std::vector<Info>& infos,
                              std::string_view str) {
        for (;;) {
            const auto pos = str.find(',');
            const auto item = str.substr(0, pos);
            for (const auto& info : infos) {
                if (item == info.m_name) {
                    if (info.m_optional)
                        return true;
                }
            }
            if (pos == std::string_view::npos)
                break;
            str.remove_prefix(pos + 1);
        }
        return false;
    }

    void generateStruct(Output& os, TypeId typeId, GenState& state) {
        const auto& typeInfo = m_typeInfos[typeId.getIndex()];
        const auto support = findSupport(typeInfo.m_name);
        if (!support)
            return;
        const auto attrs = m_ctx.getList(typeInfo.m_elem->attrs);
        const bool returnedonly = !!findAttr(attrs, returnedonlyTag);
        std::vector<MemberInfo> members;
        processChildElems(
            *typeInfo.m_elem, memberTag, [&](const Element& elem) {
                const auto attrs = m_ctx.getList(elem.attrs);
                if (!checkApi(attrs))
                    return;
                if (const auto nameTxt = getChildElemText(elem, nameTag)) {
                    const auto name = m_ctx.get(nameTxt); //
                    if (name == "pNext" || name == "matrix" ||
                        name == "ppGeometries" || name == "ppUsageCounts")
                        return;
                    auto info = getMemberInfo(elem);
                    if (returnedonly) {
                        if (info.m_isArr && !members.empty() &&
                            info.m_name.ends_with('s')) {
                            const auto prevName = members.back().m_name;
                            if (prevName.ends_with("Count") &&
                                info.m_name.substr(0, info.m_name.size() - 1) ==
                                    prevName.substr(0, prevName.size() - 5)) {
                                info.m_newType.resize(
                                    info.m_newType.size() -
                                    (info.m_array.size() + 3));
                                info.m_newType += '>';
                                info.m_array = prevName;
                                members.pop_back();
                            }
                        }
                    } else if (!info.m_optional) {
                        if (info.m_newType == "Bool32") {
                            info.m_optional = true;
                        } else if (const auto lenAttr =
                                       findAttr(attrs, lenTag)) {
                            info.m_optional =
                                isAnyOptional(members, m_ctx.get(lenAttr));
                        }
                    }
                    if (!members.empty() && info.m_name == "objectHandle" &&
                        info.m_newType == "uint64_t") {
                        auto& prev = members.back();
                        if (prev.m_name == "objectType" &&
                            prev.m_newType == "ObjectType") {
                            prev.m_slaveName = ".type";
                            prev.m_tag = VarTag::Slave;
                            info.m_slaveName = ".handle";
                            info.m_tag = VarTag::Slave;
                            members.push_back(std::move(info));
                            info.m_newType = "Object";
                            info.m_name = "object";
                            info.m_tag = VarTag::Master + 2;
                        }
                    }
                    members.push_back(std::move(info));
                }
            });
        const auto guard = updateGuard(os, *support, state);
        os << '\n';
        state.m_delim = true;
        generateGuard(os, guard);
        if (const auto commentAttr = findAttr(attrs, commentTag)) {
            os << "// " << m_ctx.get(commentAttr) << '\n';
        }
        os << "struct " << typeInfo.m_name << " : Vk" << typeInfo.m_name
           << " {\n";
        {
            os << "  " << typeInfo.m_name << "() noexcept : Vk"
               << typeInfo.m_name << '{';
            bool delim = false;
            for (const auto& member : members) {
                if (member.m_valuesAttr) {
                    if (delim)
                        os << ", ";
                    else
                        delim = true;
                    os << '.' << member.m_name << " = "
                       << m_ctx.get(member.m_valuesAttr);
                }
            }
            os << "} {}\n";
        }
        bool allRequired = false;
        if (!returnedonly) {
            allRequired = !members.empty();
            for (const auto& member : members) {
                if (member.m_optional || member.m_valuesAttr ||
                    member.m_tag == VarTag::Slave) {
                    allRequired = false;
                    break;
                }
            }
        }
        if (allRequired) {
            os << "  " << typeInfo.m_name << '(';
            bool delim = false;
            for (const auto& member : members) {
                if (delim)
                    os << ", ";
                else
                    delim = true;
                os << member.m_newType << ' ' << member.m_name;
            }
            os << ") noexcept : Vk" << typeInfo.m_name << '{';
            delim = false;
            for (const auto& member : members) {
                if (!member.m_isArr && member.m_tag == VarTag::Normal) {
                    if (delim)
                        os << ", ";
                    else
                        delim = true;
                    os << '.' << member.m_name << " = ";
                    generateMemberInit(os, member, member.m_name);
                }
            }
            os << "} {";
            bool any = false;
            for (const auto& member : members) {
                if (!member.m_isArr && member.m_tag < VarTag::Master)
                    continue;
                any = true;
                os << " set";
                generateMemberName(os, member.m_name, member.m_isPtr);
                os << '(' << member.m_name << ");";
            }
            if (any)
                os << ' ';
            os << "}\n";
        }
        std::erase_if(members,
                      [&, delim = false](const MemberInfo& member) mutable {
                          return bool(member.m_valuesAttr);
                      });
        if (!members.empty()) {
            os << '\n';
            const auto count =
                std::erase_if(members, [&](const MemberInfo& member) {
                    if (member.m_optional)
                        return false;
                    generateMember(os, member, returnedonly);
                    return true;
                });
            // optional
            if (!members.empty()) {
                if (count)
                    os << '\n';
                for (const auto& member : members) {
                    generateMember(os, member, returnedonly);
                }
            }
        }
        const auto extends = findStructExtends(typeInfo.m_name);
        if (!extends.empty()) {
            os << '\n';
            GenState stateExt;
            for (const auto p : extends) {
                const auto name =
                    m_ctx.get(findAttr(m_ctx.getList(p->attrs), nameTag))
                        .substr(2);
                const auto supportExt = findSupport(name);
                if (!supportExt)
                    continue;
                const auto guardExt = updateGuard(
                    os, subGuard(state.m_guard, *supportExt), stateExt);
                generateGuard(os, guardExt);
                os << "  void attach";
                if (m_structExtendsMap.contains(name))
                    os << "Head";
                os << "(struct " << name << "&);\n";
            }
            updateGuard(os, {}, stateExt);
        }
        os << "};\n";
        if (const auto structextendsAttr = findAttr(attrs, structextendsTag)) {
            GenState stateExt;
            for (auto structextends = m_ctx.get(structextendsAttr);;) {
                const auto pos = structextends.find(',');
                auto type = structextends.substr(0, pos);
                if (consumeMatch(type, "Vk")) {
                    if (const auto supportExt = findSupport(type)) {
                        const auto guardExt = updateGuard(
                            os, subGuard(state.m_guard, *supportExt), stateExt);
                        generateGuard(os, guardExt);
                        os << "inline void " << type << "::attach";
                        if (!extends.empty())
                            os << "Head";
                        os << '(' << typeInfo.m_name << "& ext) { ";
                        if (extends.empty())
                            os << "ext.pNext = const_cast<void*>(pNext); ";
                        os << "pNext = &ext; }\n";
                    }
                }
                if (pos == std::string_view::npos)
                    break;
                structextends.remove_prefix(pos + 1);
            }
            updateGuard(os, {}, stateExt);
        }
    }

    static bool findCommaList(std::string_view str, std::string_view value) {
        for (;;) {
            const auto pos = str.find(',');
            const auto item = str.substr(0, pos);
            if (item == value)
                return true;
            if (pos == std::string_view::npos)
                break;
            str.remove_prefix(pos + 1);
        }
        return false;
    }

    struct ParamInfo {
        std::string m_name;
        std::string m_type;
        std::string m_cast;
        bool m_addPtr = false;
        bool m_isArr = false;
        bool m_optional = false;
        std::uint8_t m_tag = Normal;
    };

    static void renamePtrName(std::string& name) {
        if (name.starts_with('p')) {
            name.erase(name.begin());
            name[0] = toLower(name[0]);
        }
    }

    ParamInfo generateParam(const Element& param, std::string_view& outType) {
        const auto attrs = m_ctx.getList(param.attrs);
        const auto optionalAttr = findAttr(attrs, optionalTag);
        auto var = getVarInfo(param);
        ParamInfo info;
        const auto type = var.m_type;
        const bool isPtr = var.m_typeSuffix.ends_with('*');
        const bool addCast =
            consumeMatch(var.m_type, "Vk") && !m_raws.contains(var.m_type);
        info.m_name = var.m_name;
        info.m_isArr = !var.m_array.empty();
        if (info.m_isArr)
            info.m_type += "std::span<";
        info.m_type += var.m_typePrefix;
        info.m_type += var.m_type;
        info.m_optional = optionalAttr && m_ctx.get(optionalAttr) == "true";
        if (isPtr && !findAttr(attrs, lenTag)) {
            if (!optionalAttr && var.m_typePrefix.starts_with("const") &&
                type != "void") {
                renamePtrName(info.m_name);
                info.m_addPtr = true;
                info.m_type += '&';
                var.m_typeSuffix = {};
            }
            if (!info.m_optional && !info.m_isArr)
                outType = type;
        }
        info.m_type += var.m_typeSuffix;
        if (info.m_isArr) {
            info.m_type += ", ";
            info.m_type += var.m_array;
            info.m_type += '>';
        }
        if (addCast) {
            info.m_cast += var.m_typePrefix;
            info.m_cast += type;
            info.m_cast += var.m_typeSuffix;
            if (info.m_addPtr || info.m_isArr)
                info.m_cast += '*';
        }
        return info;
    }

    void generateFnName(Output& os, std::string_view typeName,
                        std::string_view name) {
        if (consumeMatch(name, "Get")) {
            os << "get";
            consumeMatch(name, typeName);
        } else if (consumeMatch(name, "Destroy")) {
            os << "destroy";
            if (name == typeName) {
                name = {};
            }
        } else {
            consumeMatch(name, typeName);
            os << char(name[0] + ('a' - 'A'));
            name.remove_prefix(1);
        }
        if (name.ends_with(typeName)) {
            name.remove_suffix(typeName.size());
        }
        os << name;
    }

    void generateCommand(Output& os, const CommandInfo& cmd,
                         std::string_view typeName, StrId baseGuard,
                         GenState& state) {
        auto name = m_ctx.get(cmd.m_name);
        if (!consumeMatch(name, "vk"))
            return;
        const auto support = findSupport(name);
        if (!support)
            return;
        const auto children = m_ctx.getList(cmd.m_elem.children);
        auto childP = children.begin();
        const auto& proto = m_ctx.get(Idx<Element>{childP->getIndex()});
        auto type = m_ctx.get(getChildElemText(proto, typeTag));
        consumeMatch(type, "Vk");
        ++childP;
        if (!typeName.empty())
            ++childP;
        std::vector<ParamInfo> params;
        std::string_view outType;
        for (const auto childE = children.end(); childP != childE; ++childP) {
            const auto& param = m_ctx.get(Idx<Element>{childP->getIndex()});
            if (param.tag != paramTag)
                continue;
            const auto attrs = m_ctx.getList(param.attrs);
            if (!checkApi(attrs))
                continue;
            outType = {};
            auto info = generateParam(param, outType);
            if (const auto lenAttr = findAttr(attrs, lenTag)) {
                if (!info.m_optional)
                    info.m_optional = isAnyOptional(params, m_ctx.get(lenAttr));
            }
            if (!params.empty() && info.m_name == "objectHandle" &&
                info.m_type == "uint64_t") {
                auto& prev = params.back();
                if (prev.m_name == "objectType" &&
                    prev.m_type == "ObjectType") {
                    prev.m_name = "object.type";
                    prev.m_tag = VarTag::Slave;
                    info.m_name = "object.handle";
                    info.m_tag = VarTag::Slave;
                    params.push_back(std::move(info));
                    info.m_type = "Object";
                    info.m_name = "object";
                    info.m_tag = VarTag::Master;
                }
            }
            params.push_back(std::move(info));
        }
        ParamInfo outParam;
        bool useRet = false;
        bool useOut = false;
        if (!outType.empty()) {
            useRet = type == "Result";
            if (useRet || type == "void") {
                if (std::ranges::count(params.back().m_type, '*') > 1) {
                    useOut = true;
                    outParam = std::move(params.back());
                    if (outParam.m_cast.empty()) {
                        outType = outParam.m_type;
                    } else {
                        outType = outParam.m_cast;
                    }
                    outType.remove_suffix(1);
                } else {
                    useOut = consumeMatch(outType, "Vk")
                                 ? m_handleCommands.contains(outType) ||
                                       m_raws.contains(outType) ||
                                       m_enumOrFlag.contains(outType)
                                 : outType != "void";
                    if (useOut)
                        outParam = std::move(params.back());
                }
            }
        }
        const auto guard =
            updateGuard(os, subGuard(baseGuard, *support), state);
        if (state.m_delim) {
            os << '\n';
            state.m_delim = false;
        }
        generateGuard(os, guard);
        os << (typeName.empty() ? "inline " : "  ");
        if (useOut) {
            params.pop_back();
            if (useRet)
                os << "Ret<" << outType << "> ";
            else
                os << outType << ' ';
        } else {
            os << type << ' ';
        }
        auto lastNonOpt = params.end();
        for (const auto b = params.begin(); lastNonOpt != b;) {
            --lastNonOpt;
            if (!lastNonOpt->m_optional)
                break;
        }
        for (auto p = params.begin(); p != lastNonOpt; ++p) {
            if (p->m_optional && p->m_name != "pAllocator")
                p->m_optional = false;
        }
        generateFnName(os, typeName, name);
        os << '(';
        bool delim = false;
        for (const auto& param : params) {
            if (param.m_optional || param.m_tag == VarTag::Slave)
                continue;
            if (delim)
                os << ", ";
            else
                delim = true;
            os << param.m_type << ' ' << param.m_name;
        }
        for (const auto& param : params) {
            if (!param.m_optional || param.m_tag == VarTag::Slave)
                continue;
            if (delim)
                os << ", ";
            else
                delim = true;
            os << param.m_type << ' ' << param.m_name << " = {}";
        }
        os << ") ";
        if (!typeName.empty())
            os << "const ";
        os << "{ ";
        auto suffix = ")";
        if (useOut) {
            os << outType << " value; ";
            if (useRet) {
                os << "return {Result(";
                suffix = ")), value}";
            } else {
                suffix = "); return value";
            }
        } else if (type == "Result") {
            os << "return Result(";
            suffix = "))";
        } else if (type != "void") {
            os << "return ";
        }
        os << "vk" << name << '(';
        delim = false;
        if (!typeName.empty()) {
            os << "this->handle";
            delim = true;
        }
        for (const auto& param : params) {
            if (param.m_tag == VarTag::Master)
                continue;
            if (delim)
                os << ", ";
            else
                delim = true;
            if (!param.m_cast.empty())
                os << "std::bit_cast<" << param.m_cast << ">(";
            if (param.m_addPtr)
                os << '&';
            os << param.m_name;
            if (param.m_isArr)
                os << ".data()";
            if (!param.m_cast.empty())
                os << ')';
        }
        if (useOut) {
            if (delim)
                os << ", ";
            if (outParam.m_cast.empty())
                os << "&value";
            else
                os << "std::bit_cast<" << outParam.m_cast << ">(&value)";
        }
        os << suffix;
        os << "; }\n";
    }

    void generateHandle(Output& os, TypeId typeId, GenState& state) {
        const auto& typeInfo = m_typeInfos[typeId.getIndex()];
        const auto support = findSupport(typeInfo.m_name);
        if (!support)
            return;
        const auto guard = updateGuard(os, *support, state);
        if (state.m_delim) {
            os << '\n';
            state.m_delim = false;
        }
        generateGuard(os, guard);
        os << "struct " << typeInfo.m_name << " : Handle<Vk" << typeInfo.m_name
           << ", ObjectType::e" << typeInfo.m_name << "> {";
        GenState stateMethod{.m_delim = true};
        for (const auto& cmd : findCommands(typeInfo.m_name)) {
            generateCommand(os, cmd, typeInfo.m_name, state.m_guard,
                            stateMethod);
        }
        updateGuard(os, {}, stateMethod);
        os << "};\n";
    }

    template<class Fn>
    void processChildElems(const Element& elem, StrId tag, Fn fn) {
        for (const auto child : m_ctx.getList(elem.children)) {
            if (child.getKind() == NodeKind::Element) {
                const auto& elem = m_ctx.get(Idx<Element>{child.getIndex()});
                if (elem.tag == tag) {
                    fn(elem);
                }
            }
        }
    }

    std::string_view getText(const Element& elem) const {
        if (elem.children.count == 1) {
            const auto node = m_ctx.get(elem.children.start);
            if (node.getKind() == NodeKind::Text) {
                return m_ctx.get(StrId{node.getIndex()});
            }
        }
        return {};
    }

    StrId getChildElemText(const Element& elem, StrId tag) const {
        for (const auto child : m_ctx.getList(elem.children)) {
            if (child.getKind() == NodeKind::Element) {
                const auto& elem = m_ctx.get(Idx<Element>{child.getIndex()});
                if (elem.tag == tag) {
                    if (elem.children.count == 1) {
                        const auto node = m_ctx.get(elem.children.start);
                        if (node.getKind() == NodeKind::Text) {
                            return StrId{node.getIndex()};
                        }
                    }
                }
            }
        }
        return {};
    }

    std::string_view getTypeName(TypeId typeId) const {
        switch (typeId.getKind()) {
        case TypeKind::Alias: return m_defInfo[typeId.getIndex()].m_name;
        case TypeKind::Bitmask: return m_bitmaskInfo[typeId.getIndex()].m_name;
        default: return m_typeInfos[typeId.getIndex()].m_name;
        }
    }

    void process() {
        const auto& root = m_ctx.get(Idx<Element>{0});
        for (const auto child : m_ctx.getList(root.children)) {
            if (child.getKind() == NodeKind::Element) {
                const auto& elem = m_ctx.get(Idx<Element>{child.getIndex()});
                if (elem.tag == typesTag) {
                    processChildElems(
                        elem, typeTag, [this](const Element& elem) {
                            if (const auto categoryAttr = findAttr(
                                    m_ctx.getList(elem.attrs), categoryTag)) {
                                const auto category = m_ctx.get(categoryAttr);
                                if (category == "struct") {
                                    processStructType(elem);
                                } else if (category == "handle") {
                                    processHandleType(elem);
                                } else if (category == "basetype") {
                                    processBaseType(elem);
                                } else if (category == "union") {
                                    processUnionType(elem);
                                } else if (category == "enum") {
                                    processEnumType(elem);
                                } else if (category == "bitmask") {
                                    processBitmaskType(elem);
                                }
                            }
                        });
                } else if (elem.tag == enumsTag) {
                    const auto attrs = m_ctx.getList(elem.attrs);
                    auto name = m_ctx.get(findAttr(attrs, nameTag));
                    if (consumeMatch(name, "Vk")) {
                        m_typeIds.push_back(
                            {TypeKind::Enum, Index(m_typeInfos.size())});
                        m_typeInfos.push_back({name, &elem});
                    }
                } else if (elem.tag == tagsTag) {
                    processChildElems(
                        elem, tagTag, [this](const Element& elem) {
                            const auto attrs = m_ctx.getList(elem.attrs);
                            if (const auto nameAttr =
                                    findAttr(attrs, nameTag)) {
                                m_exts.insert(m_ctx.get(nameAttr));
                            }
                        });
                } else if (elem.tag == featureTag) {
                    const auto attrs = m_ctx.getList(elem.attrs);
                    if (!checkApi(attrs))
                        continue;
                    const auto guard = findAttr(attrs, nameTag);
                    if (const auto apitypeAttr = findAttr(attrs, apitypeTag)) {
                        if (m_ctx.get(apitypeAttr) == "internal") {
                            m_internalFeatureMap.insert(
                                {m_ctx.get(guard), &elem});
                            continue;
                        }
                    }
                    processFeature(elem, guard);
                } else if (elem.tag == extensionsTag) {
                    processChildElems(
                        elem, extensionTag, [this](const Element& elem) {
                            const auto attrs = m_ctx.getList(elem.attrs);
                            if (const auto supportedAttr =
                                    findAttr(attrs, supportedTag)) {
                                if (!findCommaList(m_ctx.get(supportedAttr),
                                                   "vulkan"))
                                    return;
                                if (findAttr(attrs, deprecatedbyTag))
                                    return;
                                if (findAttr(attrs, provisionalTag))
                                    return;
                                const auto guard = findAttr(attrs, nameTag);
                                processRequireList(elem, guard);
                            }
                        });
                } else if (elem.tag == commandsTag) {
                    processChildElems(
                        elem, commandTag,
                        [this](const Element& elem) { processCommand(elem); });
                }
            }
        }
        const auto beg = topologicalSort(m_typeIds, [this](TypeId from,
                                                           TypeId to) {
            return m_typeDeps.contains({getTypeName(from), getTypeName(to)});
        });
        const auto end = m_typeIds.end();
        for (auto it = beg; it != end; ++it) {
            const auto name = getTypeName(*it);
            print("CYC: {}\n", name);
            for (auto it2 = beg; it2 != end; ++it2) {
                const auto name2 = getTypeName(*it2);
                if (m_typeDeps.contains({name2, name})) {
                    print("  {}\n", name2);
                }
            }
        }
    }

    void processFeature(const Element& elem, StrId guard) {
        if (const auto dependsAttr =
                findAttr(m_ctx.getList(elem.attrs), dependsTag)) {
            auto str = m_ctx.get(dependsAttr);
            for (;;) {
                const auto pos = str.find(',');
                const auto item = str.substr(0, pos);
                const auto it = m_internalFeatureMap.find(item);
                if (it != m_internalFeatureMap.end()) {
                    const auto& internalDep = *it->second;
                    m_internalFeatureMap.erase(it);
                    processFeature(internalDep, guard);
                }
                if (pos == std::string_view::npos)
                    break;
                str.remove_prefix(pos + 1);
            }
        }
        processRequireList(elem, guard);
        processChildElems(elem, removeTag, [this](const Element& elem) {
            if (const auto nameAttr =
                    findAttr(m_ctx.getList(elem.attrs), nameTag)) {
                auto name = m_ctx.get(nameAttr);
                if (consumeMatch(name, "Vk") || consumeMatch(name, "vk")) {
                    m_supported.erase(name);
                }
            }
        });
    }

    void processRequireList(const Element& elem, StrId guard) {
        m_scopes.insert(m_ctx.get(guard));
        processChildElems(elem, requireTag, [this, guard](const Element& elem) {
            const auto attrs = m_ctx.getList(elem.attrs);
            if (!checkApi(attrs))
                return;
            if (const auto dependsAttr = findAttr(attrs, dependsTag)) {
                auto str = m_ctx.get(dependsAttr);
                for (;;) {
                    const auto pos = str.find(',');
                    const auto item = str.substr(0, pos);
                    if (!m_scopes.contains(item))
                        return;
                    if (pos == std::string_view::npos)
                        break;
                    str.remove_prefix(pos + 1);
                }
            }
            for (const auto child : m_ctx.getList(elem.children)) {
                if (child.getKind() == NodeKind::Element) {
                    const auto& elem =
                        m_ctx.get(Idx<Element>{child.getIndex()});
                    if (elem.tag == enumTag) {
                        if (const auto extendsAttr = findAttr(
                                m_ctx.getList(elem.attrs), extendsTag)) {
                            auto extends = m_ctx.get(extendsAttr);
                            if (consumeMatch(extends, "Vk")) {
                                m_enumExtendsMap[extends].push_back(
                                    {elem, guard});
                            }
                        }
                    } else if (elem.tag == typeTag) {
                        if (const auto nameAttr =
                                findAttr(m_ctx.getList(elem.attrs), nameTag)) {
                            auto name = m_ctx.get(nameAttr);
                            if (consumeMatch(name, "Vk")) {
                                m_supported.emplace(name, guard);
                            }
                        }
                    } else if (elem.tag == commandTag) {
                        if (const auto nameAttr =
                                findAttr(m_ctx.getList(elem.attrs), nameTag)) {
                            auto name = m_ctx.get(nameAttr);
                            if (consumeMatch(name, "vk")) {
                                m_supported.emplace(name, guard);
                            }
                        }
                    }
                }
            }
        });
    }

    void processStructType(const Element& elem) {
        const auto attrs = m_ctx.getList(elem.attrs);
        auto name = m_ctx.get(findAttr(attrs, nameTag));
        if (consumeMatch(name, "Vk")) {
            if (const auto aliasAttr = findAttr(attrs, aliasTag)) {
                processAlias(aliasAttr, name);
            } else {
                if (name.starts_with("Base")) {
                    addRaw(name);
                } else {
                    processChildElems(
                        elem, memberTag, [this, name](const Element& elem) {
                            if (const auto typeTxt =
                                    getChildElemText(elem, typeTag)) {
                                auto type = m_ctx.get(typeTxt);
                                if (consumeMatch(type, "Vk")) {
                                    if (type != name)
                                        m_typeDeps.insert({type, name});
                                }
                            }
                        });
                    m_typeIds.push_back(
                        {TypeKind::Struct, Index(m_typeInfos.size())});
                    m_structs.insert(name);
                    m_typeInfos.push_back({name, &elem});
                }
            }
        }
        if (const auto structextendsAttr = findAttr(attrs, structextendsTag)) {
            for (auto structextends = m_ctx.get(structextendsAttr);;) {
                const auto pos = structextends.find(',');
                auto type = structextends.substr(0, pos);
                if (consumeMatch(type, "Vk")) {
                    m_typeDeps.insert({type, name});
                    m_structExtendsMap[type].push_back(&elem);
                }
                if (pos == std::string_view::npos)
                    break;
                structextends.remove_prefix(pos + 1);
            }
        }
    }

    void processHandleType(const Element& elem) {
        const auto attrs = m_ctx.getList(elem.attrs);
        if (const auto nameTxt = getChildElemText(elem, nameTag)) {
            auto name = m_ctx.get(nameTxt);
            if (consumeMatch(name, "Vk")) {
                m_handleCommands.insert({name, {}});
                if (findAttr(attrs, objtypeenumTag))
                    m_typeDeps.insert({"ObjectType", name});
                m_typeIds.push_back(
                    {TypeKind::Handle, Index(m_typeInfos.size())});
                m_typeInfos.push_back({name, &elem});
            }
        } else if (const auto nameAttr = findAttr(attrs, nameTag)) {
            auto name = m_ctx.get(nameAttr);
            if (consumeMatch(name, "Vk")) {
                if (const auto aliasAttr = findAttr(attrs, aliasTag))
                    processAlias(aliasAttr, name);
            }
        }
    }

    void processBaseType(const Element& elem) {
        if (const auto nameTxt = getChildElemText(elem, nameTag)) {
            auto name = m_ctx.get(nameTxt);
            if (consumeMatch(name, "Vk"))
                addRaw(name);
        }
    }

    void processUnionType(const Element& elem) {
        const auto attrs = m_ctx.getList(elem.attrs);
        if (const auto nameAttr = findAttr(attrs, nameTag)) {
            auto name = m_ctx.get(nameAttr);
            if (consumeMatch(name, "Vk"))
                addRaw(name);
        }
    }

    void processAlias(StrId aliasAttr, std::string_view name) {
        auto alias = m_ctx.get(aliasAttr);
        if (consumeMatch(alias, "Vk")) {
            m_typeDeps.insert({alias, name});
            m_typeIds.push_back({TypeKind::Alias, Index(m_defInfo.size())});
            m_defInfo.push_back({name, alias});
        }
    }

    void processEnumType(const Element& elem) {
        const auto attrs = m_ctx.getList(elem.attrs);
        if (const auto nameAttr = findAttr(attrs, nameTag)) {
            auto name = m_ctx.get(nameAttr);
            if (consumeMatch(name, "Vk")) {
                m_enumOrFlag.insert(name);
                if (const auto aliasAttr = findAttr(attrs, aliasTag))
                    processAlias(aliasAttr, name);
            }
        }
    }

    void processBitmaskType(const Element& elem) {
        const auto attrs = m_ctx.getList(elem.attrs);
        if (!checkApi(attrs))
            return;
        if (const auto aliasAttr = findAttr(attrs, aliasTag)) {
            if (const auto nameAttr = findAttr(attrs, nameTag)) {
                auto name = m_ctx.get(nameAttr);
                if (consumeMatch(name, "Vk")) {
                    m_enumOrFlag.insert(name);
                    processAlias(aliasAttr, name);
                }
            }
            return;
        }
        auto enumTypeAttr = findAttr(attrs, bitvaluesTag);
        if (!enumTypeAttr) {
            enumTypeAttr = findAttr(attrs, requiresTag);
        }
        if (const auto nameTxt = getChildElemText(elem, nameTag)) {
            auto name = m_ctx.get(nameTxt);
            if (consumeMatch(name, "Vk")) {
                if (const auto typeTxt = getChildElemText(elem, typeTag)) {
                    auto type = m_ctx.get(typeTxt);
                    if (consumeMatch(type, "Vk")) {
                        if (enumTypeAttr) {
                            auto enumType = m_ctx.get(enumTypeAttr);
                            if (consumeMatch(enumType, "Vk")) {
                                m_enumOrFlag.insert(name);
                                m_typeDeps.insert({enumType, name});
                                m_typeIds.push_back(
                                    {TypeKind::Bitmask,
                                     Index(m_bitmaskInfo.size())});
                                m_bitmaskInfo.push_back({name, type, enumType});
                            }
                        } else {
                            m_enumOrFlag.insert(name);
                            m_typeIds.push_back({TypeKind::Bitmask,
                                                 Index(m_bitmaskInfo.size())});
                            m_bitmaskInfo.push_back({name, type});
                        }
                    }
                }
            }
        }
    }

    void processCommand(const Element& elem) {
        const auto attrs = m_ctx.getList(elem.attrs);
        if (!checkApi(attrs))
            return;
        if (const auto aliasAttr = findAttr(attrs, aliasTag)) {
            if (const auto nameAttr = findAttr(attrs, nameTag)) {
                const auto target = m_commandElemMap.find(m_ctx.get(aliasAttr));
                if (target != m_commandElemMap.end()) {
                    const auto& cmd = target->second;
                    if (cmd.m_type) {
                        auto type = m_ctx.get(cmd.m_type);
                        if (consumeMatch(type, "Vk")) {
                            const auto handle = m_handleCommands.find(type);
                            handle->second.push_back({nameAttr, cmd.m_elem});
                        }
                    } else {
                        m_globalCommands.push_back({nameAttr, cmd.m_elem});
                    }
                }
            }
            return;
        }
        const auto children = m_ctx.getList(elem.children);
        if (children.empty())
            return;
        auto childP = children.begin();
        if (childP->getKind() != NodeKind::Element)
            return;
        const auto& proto = m_ctx.get(Idx<Element>{childP->getIndex()});
        if (proto.tag != protoTag)
            return;
        ++childP;
        if (childP == children.end())
            return;
        if (childP->getKind() != NodeKind::Element)
            return;
        const auto& param = m_ctx.get(Idx<Element>{childP->getIndex()});
        if (param.tag != paramTag)
            return;
        const auto name = getChildElemText(proto, nameTag);
        const auto typeTxt = getChildElemText(param, typeTag);
        m_commandElemMap.insert({m_ctx.get(name), {typeTxt, elem}});
        std::string_view objType;
        if (typeTxt) {
            auto type = m_ctx.get(typeTxt);
            if (consumeMatch(type, "Vk")) {
                const auto handle = m_handleCommands.find(type);
                if (handle != m_handleCommands.end()) {
                    objType = type;
                    handle->second.push_back({name, elem});
                }
            }
        }
        if (objType.empty()) {
            m_globalCommands.push_back({name, elem});
            return;
        }
        for (const auto childE = children.end(); ++childP != childE;) {
            if (childP->getKind() != NodeKind::Element)
                continue;
            const auto& param = m_ctx.get(Idx<Element>{childP->getIndex()});
            if (param.tag != paramTag)
                continue;
            if (const auto typeTxt = getChildElemText(param, typeTag)) {
                auto type = m_ctx.get(typeTxt);
                if (consumeMatch(type, "Vk")) {
                    if (type != objType)
                        m_typeDeps.insert({type, objType});
                }
            }
        }
    }
};

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        print("Usage: {} <input.bin> <output.hpp>\n", argv[0]);
        return 1;
    }
    try {
        Input in{argv[1]};
        const auto& ctx = *static_cast<const XmlContext*>(in.data());
        Builder builder{ctx};
        builder.process();
        Output os{argv[2]};
        builder.generate(os);
        return 0;
    } catch (const std::exception& e) {
        print(e.what());
    }
    return 1;
}