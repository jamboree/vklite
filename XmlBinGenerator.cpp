#include <span>
#include <vector>
#include <algorithm>
#include <tinyxml2.h>
#include "XmlBin.hpp"
#include "Input.hpp"
#include "Output.hpp"
#include "Sort.hpp"

using namespace xmlbin;

struct SegmentAlloc {
    ByteOffset offset = 0;

    Segment alloc(ByteOffset align, ByteOffset size, Size count) {
        const auto alignMask = align - 1u;
        if (offset & alignMask)
            offset = (offset & ~alignMask) + align;
        const auto old = offset;
        offset += size * count;
        return {old, count};
    }

    template<class T>
    Segment alloc(Size count) {
        return alloc(alignof(T), sizeof(T), count);
    }

    template<class T>
    Segment allocFor(const std::vector<T>& list) {
        return alloc<T>(Size(list.size()));
    }
};

template<class T>
void writeList(Output& os, const std::vector<T>& list) {
    os.write(list.data(), sizeof(T) * list.size());
}

template<class T>
std::span<T> getList(std::vector<T>& list, Sequence<T> seq) {
    return {list.data() + seq.start.value, seq.count};
}

struct Builder {
    std::vector<const char*> strings;
    std::vector<UniqueStrEntry> uniqueStrings;
    std::vector<NodeId> nodes;
    std::vector<Attribute> attrs;
    std::vector<Element> elems;
    Size stringSize = 1;

    StrId getStr(std::string_view str) {
        strings.push_back(str.data());
        const auto id = stringSize;
        stringSize += Size(str.size()) + 1;
        return {id};
    }

    StrId getUniqueStr(std::string_view str) {
        const auto getStr = [this](UniqueStrEntry entry) {
            return strings[entry.idx];
        };
        auto pos = std::ranges::lower_bound(uniqueStrings, str,
                                            std::ranges::less{}, getStr);
        if (pos == uniqueStrings.end() || str != getStr(*pos)) {
            pos =
                uniqueStrings.insert(pos, {Index(strings.size()), stringSize});
            strings.push_back(str.data());
            stringSize += Size(str.size()) + 1;
        }
        return {pos->offset};
    }

    Attribute buildAttr(const tinyxml2::XMLAttribute& attr) {
        Attribute a;
        a.name = getUniqueStr(attr.Name());
        a.value = getStr(attr.Value());
        return a;
    }

    void buildElem(const tinyxml2::XMLElement& elem) {
        Element e;
        e.tag = getUniqueStr(elem.Name());
        std::vector<Attribute> attrList;
        for (auto p = elem.FirstAttribute(); p; p = p->Next()) {
            attrList.push_back(buildAttr(*p));
        }
        std::ranges::sort(attrList, std::ranges::less{},
                          [](Attribute& a) { return a.name; });
        e.attrs = {Index(attrs.size()), Size(attrList.size())};
        attrs.resize(attrs.size() + attrList.size());
        eytzinger(e.attrs.count, [in = attrList.data(),
                                  out = attrs.data() + e.attrs.start.value](
                                     unsigned k) mutable { out[k] = *in++; });

        auto childIdx = Index(nodes.size());
        e.children.start = {childIdx};
        for (auto p = elem.FirstChild(); p; p = p->NextSibling())
            ++e.children.count;
        nodes.resize(childIdx + e.children.count);
        elems.push_back(e);

        for (auto p = elem.FirstChild(); p; p = p->NextSibling()) {
            auto& node = nodes[childIdx++];
            if (auto child = p->ToElement()) {
                node = NodeId(NodeKind::Element, Index(elems.size()));
                buildElem(*child);
            } else if (p->ToText()) {
                node = NodeId(NodeKind::Text, getStr(p->Value()).value);
            }
        }
    }

    void generate(Output& os) const {
        std::vector<StrId> uniqueStrList(uniqueStrings.size() + 1);
        eytzinger(Size(uniqueStrings.size()),
                  [in = uniqueStrings.data(), out = uniqueStrList.data() + 1](
                      unsigned k) mutable { out[k] = {(in++)->offset}; });
        Context ctx;
        {
            SegmentAlloc a{sizeof(ctx)};
            ctx.strings = a.alloc<char>(stringSize);
            ctx.uniqueStrings = a.allocFor(uniqueStrList);
            ctx.nodes = a.allocFor(nodes);
            ctx.attrs = a.allocFor(attrs);
            ctx.elems = a.allocFor(elems);
        }
        os.write(&ctx, sizeof(ctx));
        os.seek(ctx.strings.offset);
        os.write("", 1);
        for (const std::string_view s : strings) {
            os.write(s.data(), s.size() + 1);
        }
        os.seek(ctx.uniqueStrings.offset);
        writeList(os, uniqueStrList);
        os.seek(ctx.nodes.offset);
        writeList(os, nodes);
        os.seek(ctx.attrs.offset);
        writeList(os, attrs);
        os.seek(ctx.elems.offset);
        writeList(os, elems);
    }
};

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        print("usage: {} <input.xml> <output.bin>\n", argv[0]);
        return 1;
    }
    try {
        Input in{argv[1]};
        tinyxml2::XMLDocument doc;
        if (doc.Parse(static_cast<const char*>(in.data()), in.size()) !=
            tinyxml2::XML_SUCCESS) {
            print("failed to parse {}\n", argv[1]);
            return 1;
        }
        const auto root = doc.RootElement();
        if (!root) {
            print("failed to retrieve root node.\n");
            return 1;
        }
        Builder ctx;
        ctx.buildElem(*root);
        Output os{argv[2]};
        ctx.generate(os);
        return 0;
    } catch (const std::exception& e) {
        print(e.what());
    }
    return 1;
}