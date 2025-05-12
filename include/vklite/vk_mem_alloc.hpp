#ifndef VKLITE_VK_MEM_ALLOC_HPP
#define VKLITE_VK_MEM_ALLOC_HPP

#include "vulkan.hpp"
#include <vma/vk_mem_alloc.h>

namespace vklite::vma {
    template<class T>
    struct Handle {
        T handle = {};

        explicit operator bool() const noexcept { return !!handle; }
    };

    enum class AllocatorCreateFlagBits : uint32_t {
        bExternallySynchronized =
            VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
        bDedicatedAllocationKHR =
            VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT,
        bBindMemory2KHR = VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT,
        bMemoryBudgetEXT = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
        bDeviceCoherentMemoryAMD =
            VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT,
        bBufferDeviceAddress = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        bMemoryPriorityEXT = VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT,
    };

    using AllocatorCreateFlags = FlagSet<AllocatorCreateFlagBits, Flags>;
    constexpr AllocatorCreateFlags
    operator|(AllocatorCreateFlagBits a, AllocatorCreateFlagBits b) noexcept {
        return AllocatorCreateFlags(Flags(a) | Flags(b));
    }

    enum class MemoryUsage : int32_t {
        eUnknown = VMA_MEMORY_USAGE_UNKNOWN,
        eGpuLazilyAllocated = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED,
        eAuto = VMA_MEMORY_USAGE_AUTO,
        eAutoPreferDevice = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        eAutoPreferHost = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
    };

    enum class AllocationCreateFlagBits : uint32_t {
        bDedicatedMemory = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        bNeverAllocate = VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT,
        bMapped = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        bUserDataCopyString = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT,
        bUpperAddress = VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT,
        bDontBind = VMA_ALLOCATION_CREATE_DONT_BIND_BIT,
        bWithinBudget = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT,
        bCanAlias = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT,
        bHostAccessSequentialWrite =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        bHostAccessRandom = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
        bHostAccessAllowTransferInstead =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT,
        bStrategyMinMemory = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT,
        bStrategyMinTime = VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT,
        bStrategyMinOffset = VMA_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT,
        bStrategyBestFit = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT,
        bStrategyFirstFit = VMA_ALLOCATION_CREATE_STRATEGY_FIRST_FIT_BIT,
        eStrategyMask = VMA_ALLOCATION_CREATE_STRATEGY_MASK,
    };

    using AllocationCreateFlags = FlagSet<AllocationCreateFlagBits, Flags>;
    constexpr AllocationCreateFlags
    operator|(AllocationCreateFlagBits a, AllocationCreateFlagBits b) noexcept {
        return AllocationCreateFlags(Flags(a) | Flags(b));
    }

    enum class PoolCreateFlagBits : uint32_t {
        bIgnoreBufferImageGranularity =
            VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT,
        bLinearAlgorithm = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT,
        eAlgorithmMask = VMA_POOL_CREATE_ALGORITHM_MASK,
    };

    using PoolCreateFlags = FlagSet<PoolCreateFlagBits, Flags>;
    constexpr PoolCreateFlags operator|(PoolCreateFlagBits a,
                                        PoolCreateFlagBits b) noexcept {
        return PoolCreateFlags(Flags(a) | Flags(b));
    }

    enum class DefragmentationFlagBits : uint32_t {
        bAlgorithmFast = VMA_DEFRAGMENTATION_FLAG_ALGORITHM_FAST_BIT,
        bAlgorithmBalanced = VMA_DEFRAGMENTATION_FLAG_ALGORITHM_BALANCED_BIT,
        bAlgorithmFull = VMA_DEFRAGMENTATION_FLAG_ALGORITHM_FULL_BIT,
        bAlgorithmExtensive = VMA_DEFRAGMENTATION_FLAG_ALGORITHM_EXTENSIVE_BIT,
        eAlgorithmMask = VMA_DEFRAGMENTATION_FLAG_ALGORITHM_MASK,
    };

    using DefragmentationFlags = FlagSet<DefragmentationFlagBits, Flags>;
    constexpr DefragmentationFlags
    operator|(DefragmentationFlagBits a, DefragmentationFlagBits b) noexcept {
        return DefragmentationFlags(Flags(a) | Flags(b));
    }

    enum class DefragmentationMoveOperation : int32_t {
        eCopy = VMA_DEFRAGMENTATION_MOVE_OPERATION_COPY,
        eIgnore = VMA_DEFRAGMENTATION_MOVE_OPERATION_IGNORE,
        eDestroy = VMA_DEFRAGMENTATION_MOVE_OPERATION_DESTROY,
    };

    enum class VirtualBlockCreateFlagBits : uint32_t {
        bLinearAlgorithm = VMA_VIRTUAL_BLOCK_CREATE_LINEAR_ALGORITHM_BIT,
        eAlgorithmMask = VMA_VIRTUAL_BLOCK_CREATE_ALGORITHM_MASK,
    };

    using VirtualBlockCreateFlags = FlagSet<VirtualBlockCreateFlagBits, Flags>;
    constexpr VirtualBlockCreateFlags
    operator|(VirtualBlockCreateFlagBits a,
              VirtualBlockCreateFlagBits b) noexcept {
        return VirtualBlockCreateFlags(Flags(a) | Flags(b));
    }

    enum class VirtualAllocationCreateFlagBits : uint32_t {
        bUpperAddress = VMA_VIRTUAL_ALLOCATION_CREATE_UPPER_ADDRESS_BIT,
        bStrategyMinMemory =
            VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT,
        bStrategyMinTime = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT,
        bStrategyMinOffset =
            VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT,
        eStrategyMask = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MASK,
    };

    using VirtualAllocationCreateFlags =
        FlagSet<VirtualAllocationCreateFlagBits, Flags>;
    constexpr VirtualAllocationCreateFlags
    operator|(VirtualAllocationCreateFlagBits a,
              VirtualAllocationCreateFlagBits b) noexcept {
        return VirtualAllocationCreateFlags(Flags(a) | Flags(b));
    }

    struct AllocatorCreateInfo : VmaAllocatorCreateInfo {
        AllocatorCreateInfo() noexcept : VmaAllocatorCreateInfo{} {}

        AllocatorCreateInfo(Instance instance, PhysicalDevice physicalDevice,
                            Device device) noexcept
            : VmaAllocatorCreateInfo{
                  .physicalDevice =
                      std::bit_cast<VkPhysicalDevice>(physicalDevice),
                  .device = std::bit_cast<VkDevice>(device),
                  .instance = std::bit_cast<VkInstance>(instance)} {}

        void setInstance(Instance value) {
            instance = std::bit_cast<VkInstance>(value);
        }
        Instance getInstance() const {
            return std::bit_cast<Instance>(instance);
        }
        void setPhysicalDevice(PhysicalDevice value) {
            physicalDevice = std::bit_cast<VkPhysicalDevice>(value);
        }
        PhysicalDevice getPhysicalDevices() const {
            return std::bit_cast<PhysicalDevice>(physicalDevice);
        }
        void setDevice(Device value) {
            device = std::bit_cast<VkDevice>(value);
        }
        Device getDevice() const { return std::bit_cast<Device>(device); }

        void setFlags(AllocatorCreateFlags value) {
            flags = std::bit_cast<VmaAllocatorCreateFlags>(value);
        }
        AllocatorCreateFlags getFlags() const {
            return std::bit_cast<AllocatorCreateFlags>(flags);
        }
        void setPreferredLargeHeapBlockSize(DeviceSize value) {
            preferredLargeHeapBlockSize = value;
        }
        DeviceSize getPreferredLargeHeapBlockSize() const {
            return preferredLargeHeapBlockSize;
        }
        void setAllocationCallbacks(const VkAllocationCallbacks* value) {
            pAllocationCallbacks = value;
        }
        const VkAllocationCallbacks* getAllocationCallbacks() const {
            return pAllocationCallbacks;
        }
        void setDeviceMemoryCallbacks(const VmaDeviceMemoryCallbacks* value) {
            pDeviceMemoryCallbacks = value;
        }
        const VmaDeviceMemoryCallbacks* getDeviceMemoryCallbacks() const {
            return pDeviceMemoryCallbacks;
        }
        void setHeapSizeLimit(const DeviceSize* value) {
            pHeapSizeLimit = value;
        }
        const DeviceSize* getHeapSizeLimit() const { return pHeapSizeLimit; }
        const VmaVulkanFunctions* getVulkanFunctions() const {
            return pVulkanFunctions;
        }
        void setVulkanFunctions(const VmaVulkanFunctions* value) {
            pVulkanFunctions = value;
        }
        uint32_t getVulkanApiVersion() const { return vulkanApiVersion; }
        void setVulkanApiVersion(uint32_t value) { vulkanApiVersion = value; }
#if VMA_EXTERNAL_MEMORY
        void setTypeExternalMemoryHandleTypes(
            const ExternalMemoryHandleTypeFlagsKHR* value) {
            pTypeExternalMemoryHandleTypes =
                std::bit_cast<const VkExternalMemoryHandleTypeFlagsKHR*>(value);
        }
        const ExternalMemoryHandleTypeFlagsKHR*
        getTypeExternalMemoryHandleTypes() const {
            return std::bit_cast<const ExternalMemoryHandleTypeFlagsKHR*>(
                pTypeExternalMemoryHandleTypes);
        }
#endif
    };

    struct AllocatorInfo : VmaAllocatorInfo {
        AllocatorInfo() noexcept : VmaAllocatorInfo{} {}

        AllocatorInfo(Instance instance, PhysicalDevice physicalDevice,
                      Device device) noexcept
            : VmaAllocatorInfo{
                  .instance = std::bit_cast<VkInstance>(instance),
                  .physicalDevice =
                      std::bit_cast<VkPhysicalDevice>(physicalDevice),
                  .device = std::bit_cast<VkDevice>(device),
              } {}

        void setInstance(Instance value) {
            instance = std::bit_cast<VkInstance>(value);
        }
        Instance getInstance() const {
            return std::bit_cast<Instance>(instance);
        }
        void setPhysicalDevice(PhysicalDevice value) {
            physicalDevice = std::bit_cast<VkPhysicalDevice>(value);
        }
        PhysicalDevice getPhysicalDevices() const {
            return std::bit_cast<PhysicalDevice>(physicalDevice);
        }
        void setDevice(Device value) {
            device = std::bit_cast<VkDevice>(value);
        }
        Device getDevice() const { return std::bit_cast<Device>(device); }
    };

    struct Statistics : VmaStatistics {
        Statistics() noexcept : VmaStatistics{} {}

        uint32_t getBlockCount() const { return blockCount; }
        uint32_t getAllocationCount() const { return allocationCount; }
        DeviceSize getBlockBytes() const { return blockBytes; }
        DeviceSize getAllocationBytes() const { return allocationBytes; }
    };

    using DetailedStatistics = VmaDetailedStatistics;
    using TotalStatistics = VmaTotalStatistics;

    struct Budget : VmaBudget {
        Budget() noexcept : VmaBudget{} {}

        const Statistics& getStatistics() const {
            return static_cast<const Statistics&>(statistics);
        }
        DeviceSize getUsage() const { return usage; }
        DeviceSize getBudget() const { return budget; }
    };

    struct PoolCreateInfo : VmaPoolCreateInfo {
        PoolCreateInfo() noexcept : VmaPoolCreateInfo{} {}

        PoolCreateInfo(uint32_t memoryTypeIndex) noexcept
            : VmaPoolCreateInfo{.memoryTypeIndex = memoryTypeIndex} {}

        uint32_t getMemoryTypeIndex() const { return memoryTypeIndex; }
        void setMemoryTypeIndex(uint32_t value) { memoryTypeIndex = value; }
        void setFlags(PoolCreateFlags value) {
            flags = std::bit_cast<VmaPoolCreateFlags>(value);
        }
        PoolCreateFlags getFlags() const {
            return std::bit_cast<PoolCreateFlags>(flags);
        }
        DeviceSize getBlockSize() const { return blockSize; }
        void setBlockSize(DeviceSize value) { blockSize = value; }
        size_t getMinBlockCount() const { return minBlockCount; }
        void setMinBlockCount(size_t value) { minBlockCount = value; }
        size_t getMaxBlockCount() const { return maxBlockCount; }
        void setMaxBlockCount(size_t value) { maxBlockCount = value; }
        float getPriority() const { return priority; }
        void setPriority(float value) { priority = value; }
        DeviceSize getMinAllocationAlignment() const {
            return minAllocationAlignment;
        }
        void setMinAllocationAlignment(DeviceSize value) {
            minAllocationAlignment = value;
        }

        void attach(struct ExportMemoryAllocateInfo& ext) {
            ext.pNext = pMemoryAllocateNext;
            pMemoryAllocateNext = &ext;
        }
        void attach(struct ImportMemoryFdInfoKHR& ext) {
            ext.pNext = pMemoryAllocateNext;
            pMemoryAllocateNext = &ext;
        }
        void attach(struct MemoryAllocateFlagsInfo& ext) {
            ext.pNext = pMemoryAllocateNext;
            pMemoryAllocateNext = &ext;
        }
        void attach(struct MemoryDedicatedAllocateInfo& ext) {
            ext.pNext = pMemoryAllocateNext;
            pMemoryAllocateNext = &ext;
        }
        void attach(struct ImportMemoryHostPointerInfoEXT& ext) {
            ext.pNext = pMemoryAllocateNext;
            pMemoryAllocateNext = &ext;
        }
        void attach(struct MemoryPriorityAllocateInfoEXT& ext) {
            ext.pNext = pMemoryAllocateNext;
            pMemoryAllocateNext = &ext;
        }
        void attach(struct MemoryOpaqueCaptureAddressAllocateInfo& ext) {
            ext.pNext = pMemoryAllocateNext;
            pMemoryAllocateNext = &ext;
        }
    };

    struct Pool : Handle<VmaPool> {};

    struct AllocationCreateInfo : VmaAllocationCreateInfo {
        AllocationCreateInfo() noexcept : VmaAllocationCreateInfo{} {}

        void setFlags(AllocationCreateFlags value) {
            flags = std::bit_cast<VmaAllocationCreateFlags>(value);
        }
        AllocationCreateFlags getFlags() const {
            return std::bit_cast<AllocationCreateFlags>(flags);
        }
        void setUsage(MemoryUsage value) {
            usage = std::bit_cast<VmaMemoryUsage>(value);
        }
        MemoryUsage getUsage() const {
            return std::bit_cast<MemoryUsage>(usage);
        }
        void setRequiredFlags(MemoryPropertyFlags value) {
            requiredFlags = std::bit_cast<VkMemoryPropertyFlags>(value);
        }
        MemoryPropertyFlags getRequiredFlags() const {
            return std::bit_cast<MemoryPropertyFlags>(requiredFlags);
        }
        void setPreferredFlags(MemoryPropertyFlags value) {
            preferredFlags = std::bit_cast<VkMemoryPropertyFlags>(value);
        }
        MemoryPropertyFlags getPreferredFlags() const {
            return std::bit_cast<MemoryPropertyFlags>(preferredFlags);
        }
        uint32_t getMemoryTypeBits() const { return memoryTypeBits; }
        void setMemoryTypeBits(uint32_t value) { memoryTypeBits = value; }
        void setPool(Pool value) { pool = std::bit_cast<VmaPool>(value); }
        Pool getPool() const { return std::bit_cast<Pool>(pool); }
        void* getUserData() const { return pUserData; }
        void setUserData(void* value) { pUserData = value; }
        float getPriority() const { return priority; }
        void setPriority(float value) { priority = value; }
    };

    struct AllocationInfo : VmaAllocationInfo {
        AllocationInfo() noexcept : VmaAllocationInfo{} {}

        uint32_t getMemoryType() const { return memoryType; }
        DeviceMemory getDeviceMemory() const {
            return std::bit_cast<DeviceMemory>(deviceMemory);
        }
        DeviceSize getOffset() const { return offset; }
        DeviceSize getSize() const { return size; }
        void* getMappedData() const { return pMappedData; }
        void* getUserData() const { return pUserData; }
        std::string_view getName() const { return pName; }
    };

    struct DefragmentationInfo : VmaDefragmentationInfo {
        DefragmentationInfo() noexcept : VmaDefragmentationInfo{} {}

        void setFlags(DefragmentationFlags value) {
            flags = std::bit_cast<VmaDefragmentationFlags>(value);
        }
        DefragmentationFlags getFlags() const {
            return std::bit_cast<DefragmentationFlags>(flags);
        }
        void setPool(Pool value) { pool = std::bit_cast<VmaPool>(value); }
        Pool getPool() const { return std::bit_cast<Pool>(pool); }
        void setMaxBytesPerPass(DeviceSize value) { maxBytesPerPass = value; }
        DeviceSize getMaxBytesPerPass() const { return maxBytesPerPass; }
        uint32_t getMaxAllocationsPerPass() const {
            return maxAllocationsPerPass;
        }
        void setMaxAllocationsPerPass(uint32_t value) {
            maxAllocationsPerPass = value;
        }
    };

    struct Allocation : Handle<VmaAllocation> {};

    struct DefragmentationMove : VmaDefragmentationMove {
        DefragmentationMove() noexcept : VmaDefragmentationMove{} {}

        DefragmentationMove(Allocation srcAllocation,
                            Allocation dstTmpAllocation) noexcept
            : VmaDefragmentationMove{
                  .srcAllocation = std::bit_cast<VmaAllocation>(srcAllocation),
                  .dstTmpAllocation =
                      std::bit_cast<VmaAllocation>(dstTmpAllocation)} {}

        void setSrcAllocation(Allocation value) {
            srcAllocation = std::bit_cast<VmaAllocation>(value);
        }
        Allocation getSrcAllocation() const {
            return std::bit_cast<Allocation>(srcAllocation);
        }
        void setDstTmpAllocation(Allocation value) {
            dstTmpAllocation = std::bit_cast<VmaAllocation>(value);
        }
        Allocation getDstTmpAllocation() const {
            return std::bit_cast<Allocation>(dstTmpAllocation);
        }

        void setOperation(DefragmentationMoveOperation value) {
            operation = std::bit_cast<VmaDefragmentationMoveOperation>(value);
        }
        DefragmentationMoveOperation getOperation() const {
            return std::bit_cast<DefragmentationMoveOperation>(operation);
        }
    };

    struct DefragmentationPassMoveInfo : VmaDefragmentationPassMoveInfo {
        DefragmentationPassMoveInfo() noexcept
            : VmaDefragmentationPassMoveInfo{} {}

        uint32_t getMoveCount() const { return moveCount; }
        void setMoveCount(uint32_t value) { moveCount = value; }
        void setMoves(DefragmentationMove* value) {
            pMoves = std::bit_cast<VmaDefragmentationMove*>(value);
        }
        DefragmentationMove* getMoves() const {
            return std::bit_cast<DefragmentationMove*>(pMoves);
        }
    };

    struct DefragmentationStats : VmaDefragmentationStats {
        DefragmentationStats() noexcept : VmaDefragmentationStats{} {}

        DeviceSize getBytesMoved() const { return bytesMoved; }
        DeviceSize getBytesFreed() const { return bytesFreed; }
        uint32_t getAllocationsMoved() const { return allocationsMoved; }
        uint32_t getDeviceMemoryBlocksFreed() const {
            return deviceMemoryBlocksFreed;
        }
    };

    struct VirtualBlockCreateInfo : VmaVirtualBlockCreateInfo {
        VirtualBlockCreateInfo() noexcept : VmaVirtualBlockCreateInfo{} {}

        VirtualBlockCreateInfo(DeviceSize size) noexcept
            : VmaVirtualBlockCreateInfo{.size = size} {}

        void setSize(DeviceSize value) { size = value; }
        DeviceSize getSize() const { return size; }

        void setFlags(VirtualBlockCreateFlags value) {
            flags = std::bit_cast<VmaVirtualBlockCreateFlags>(value);
        }
        VirtualBlockCreateFlags getFlags() const {
            return std::bit_cast<VirtualBlockCreateFlags>(flags);
        }
        const VkAllocationCallbacks* getAllocationCallbacks() const {
            return pAllocationCallbacks;
        }
        void setAllocationCallbacks(const VkAllocationCallbacks* value) {
            pAllocationCallbacks = value;
        }
    };

    struct VirtualAllocationCreateInfo : VmaVirtualAllocationCreateInfo {
        VirtualAllocationCreateInfo() noexcept
            : VmaVirtualAllocationCreateInfo{} {}

        VirtualAllocationCreateInfo(DeviceSize size) noexcept
            : VmaVirtualAllocationCreateInfo{.size = size} {}

        void setSize(DeviceSize value) { size = value; }
        DeviceSize getSize() const { return size; }

        void setAlignment(DeviceSize value) { alignment = value; }
        DeviceSize getAlignment() const { return alignment; }
        void setFlags(VirtualAllocationCreateFlags value) {
            flags = std::bit_cast<VmaVirtualAllocationCreateFlags>(value);
        }
        VirtualAllocationCreateFlags getFlags() const {
            return std::bit_cast<VirtualAllocationCreateFlags>(flags);
        }
        void* getUserData() const { return pUserData; }
        void setUserData(void* value) { pUserData = value; }
    };

    struct VirtualAllocationInfo : VmaVirtualAllocationInfo {
        VirtualAllocationInfo() noexcept : VmaVirtualAllocationInfo{} {}

        VirtualAllocationInfo(DeviceSize offset, DeviceSize size) noexcept
            : VmaVirtualAllocationInfo{.offset = offset, .size = size} {}

        void setOffset(DeviceSize value) { offset = value; }
        DeviceSize getOffset() const { return offset; }
        void setSize(DeviceSize value) { size = value; }
        DeviceSize getSize() const { return size; }

        void* getUserData() const { return pUserData; }
        void setUserData(void* value) { pUserData = value; }
    };

    struct DefragmentationContext : Handle<VmaDefragmentationContext> {};

    struct Allocator : Handle<VmaAllocator> {
        void destroy() const { vmaDestroyAllocator(handle); }
        void getAllocatorInfo(AllocatorInfo* pAllocatorInfo) const {
            vmaGetAllocatorInfo(handle, pAllocatorInfo);
        }
        const PhysicalDeviceProperties* getPhysicalDeviceProperties() const {
            const PhysicalDeviceProperties* value;
            vmaGetPhysicalDeviceProperties(
                handle,
                std::bit_cast<const VkPhysicalDeviceProperties**>(&value));
            return value;
        }
        const PhysicalDeviceMemoryProperties* getMemoryProperties() const {
            const PhysicalDeviceMemoryProperties* value;
            vmaGetMemoryProperties(
                handle, std::bit_cast<const VkPhysicalDeviceMemoryProperties**>(
                            &value));
            return value;
        }
        MemoryPropertyFlags
        getMemoryTypeProperties(uint32_t memoryTypeIndex) const {
            MemoryPropertyFlags value;
            vmaGetMemoryTypeProperties(
                handle, memoryTypeIndex,
                std::bit_cast<VkMemoryPropertyFlags*>(&value));
            return value;
        }
        void setCurrentFrameIndex(uint32_t frameIndex) const {
            vmaSetCurrentFrameIndex(handle, frameIndex);
        }
        void calculateStatistics(TotalStatistics* pStats) const {
            vmaCalculateStatistics(handle, pStats);
        }
        void getHeapBudgets(Budget* pBudgets) const {
            vmaGetHeapBudgets(handle, pBudgets);
        }
        Ret<uint32_t> findMemoryTypeIndex(
            uint32_t memoryTypeBits,
            const AllocationCreateInfo& allocationCreateInfo) const {
            uint32_t value;
            return {Result(vmaFindMemoryTypeIndex(
                        handle, memoryTypeBits, &allocationCreateInfo, &value)),
                    value};
        }
        Ret<uint32_t> findMemoryTypeIndexForBufferInfo(
            const BufferCreateInfo& bufferCreateInfo,
            const AllocationCreateInfo& allocationCreateInfo) const {
            uint32_t value;
            return {
                Result(vmaFindMemoryTypeIndexForBufferInfo(
                    handle, &bufferCreateInfo, &allocationCreateInfo, &value)),
                value};
        }

        Ret<uint32_t> findMemoryTypeIndexForBufferInfo(
            const ImageCreateInfo& imageCreateInfo,
            const AllocationCreateInfo& allocationCreateInfo) const {
            uint32_t value;
            return {
                Result(vmaFindMemoryTypeIndexForImageInfo(
                    handle, &imageCreateInfo, &allocationCreateInfo, &value)),
                value};
        }
        Ret<Pool> createPool(const PoolCreateInfo& createInfo) {
            Pool value;
            return {Result(vmaCreatePool(handle, &createInfo,
                                         std::bit_cast<VmaPool*>(&value))),
                    value};
        }
        void destroyPool(Pool pool) const {
            vmaDestroyPool(handle, std::bit_cast<VmaPool>(pool));
        }
        void getPoolStatistics(Pool pool, Statistics* pPoolStats) const {
            vmaGetPoolStatistics(handle, std::bit_cast<VmaPool>(pool),
                                 pPoolStats);
        }
        void calculatePoolStatistics(Pool pool,
                                     DetailedStatistics* pPoolStats) const {
            vmaCalculatePoolStatistics(handle, std::bit_cast<VmaPool>(pool),
                                       pPoolStats);
        }
        Result checkPoolCorruption(Pool pool) const {
            return Result(
                vmaCheckPoolCorruption(handle, std::bit_cast<VmaPool>(pool)));
        }
        const char* getPoolName(Pool pool) const {
            const char* value;
            vmaGetPoolName(handle, std::bit_cast<VmaPool>(pool), &value);
            return value;
        }
        void setPoolName(Pool pool, const char* pName = {}) const {
            vmaSetPoolName(handle, std::bit_cast<VmaPool>(pool), pName);
        }
        Ret<Allocation>
        allocateMemory(const MemoryRequirements& memoryRequirements,
                       const AllocationCreateInfo& createInfo,
                       AllocationInfo* pAllocationInfo = {}) const {
            Allocation value;
            return {
                Result(vmaAllocateMemory(
                    handle, &memoryRequirements, &createInfo,
                    std::bit_cast<VmaAllocation*>(&value), pAllocationInfo)),
                value};
        }
        Ret<Allocation>
        allocateMemoryPages(const MemoryRequirements& memoryRequirements,
                            const AllocationCreateInfo& createInfo,
                            size_t allocationCount,
                            AllocationInfo* pAllocationInfo = {}) const {
            Allocation value;
            return {
                Result(vmaAllocateMemoryPages(
                    handle, &memoryRequirements, &createInfo, allocationCount,
                    std::bit_cast<VmaAllocation*>(&value), pAllocationInfo)),
                value};
        }
        Ret<Allocation>
        allocateMemoryForBuffer(Buffer buffer,
                                const AllocationCreateInfo& createInfo,
                                AllocationInfo* pAllocationInfo = {}) const {
            Allocation value;
            return {
                Result(vmaAllocateMemoryForBuffer(
                    handle, std::bit_cast<VkBuffer>(buffer), &createInfo,
                    std::bit_cast<VmaAllocation*>(&value), pAllocationInfo)),
                value};
        }
        Ret<Allocation>
        allocateMemoryForImage(Image image,
                               const AllocationCreateInfo& createInfo,
                               AllocationInfo* pAllocationInfo = {}) const {
            Allocation value;
            return {
                Result(vmaAllocateMemoryForImage(
                    handle, std::bit_cast<VkImage>(image), &createInfo,
                    std::bit_cast<VmaAllocation*>(&value), pAllocationInfo)),
                value};
        }
        void freeMemory(Allocation allocation) const {
            vmaFreeMemory(handle, std::bit_cast<VmaAllocation>(allocation));
        }
        void freeMemoryPages(size_t allocationCount,
                             const Allocation* pAllocations) const {
            vmaFreeMemoryPages(
                handle, allocationCount,
                std::bit_cast<const VmaAllocation*>(pAllocations));
        }
        void getAllocationInfo(Allocation allocation,
                               AllocationInfo* pAllocationInfo) const {
            vmaGetAllocationInfo(handle,
                                 std::bit_cast<VmaAllocation>(allocation),
                                 pAllocationInfo);
        }
        void setAllocationUserData(Allocation allocation,
                                   void* pUserData = {}) const {
            vmaSetAllocationUserData(
                handle, std::bit_cast<VmaAllocation>(allocation), pUserData);
        }
        void setAllocationName(Allocation allocation,
                               const char* pName = {}) const {
            vmaSetAllocationName(
                handle, std::bit_cast<VmaAllocation>(allocation), pName);
        }
        MemoryPropertyFlags
        getAllocationMemoryProperties(Allocation allocation) const {
            MemoryPropertyFlags value;
            vmaGetAllocationMemoryProperties(
                handle, std::bit_cast<VmaAllocation>(allocation),
                std::bit_cast<VkMemoryPropertyFlags*>(&value));
            return value;
        }
        Ret<void*> mapMemory(Allocation allocation) const {
            void* value;
            return {
                Result(vmaMapMemory(
                    handle, std::bit_cast<VmaAllocation>(allocation), &value)),
                value};
        }
        void unmapMemory(Allocation allocation) const {
            vmaUnmapMemory(handle, std::bit_cast<VmaAllocation>(allocation));
        }
        Result flushAllocation(Allocation allocation, DeviceSize offset,
                               DeviceSize size) const {
            return Result(vmaFlushAllocation(
                handle, std::bit_cast<VmaAllocation>(allocation), offset,
                size));
        }
        Result invalidateAllocation(Allocation allocation, DeviceSize offset,
                                    DeviceSize size) const {
            return Result(vmaInvalidateAllocation(
                handle, std::bit_cast<VmaAllocation>(allocation), offset,
                size));
        }
        Result flushAllocations(uint32_t allocationCount,
                                const Allocation* allocations,
                                const VkDeviceSize* offsets = {},
                                const VkDeviceSize* sizes = {}) const {
            return Result(vmaFlushAllocations(
                handle, allocationCount,
                std::bit_cast<const VmaAllocation*>(allocations), offsets,
                sizes));
        }
        Result invalidateAllocations(uint32_t allocationCount,
                                     const Allocation* allocations,
                                     const VkDeviceSize* offsets = {},
                                     const VkDeviceSize* sizes = {}) const {
            return Result(vmaInvalidateAllocations(
                handle, allocationCount,
                std::bit_cast<const VmaAllocation*>(allocations), offsets,
                sizes));
        }
        Result checkCorruption(uint32_t memoryTypeBits) const {
            return Result(vmaCheckCorruption(handle, memoryTypeBits));
        }
        Ret<DefragmentationContext>
        beginDefragmentation(const DefragmentationInfo& info) const {
            DefragmentationContext value;
            return {Result(vmaBeginDefragmentation(
                        handle, &info,
                        std::bit_cast<VmaDefragmentationContext*>(&value))),
                    value};
        }
        void endDefragmentation(DefragmentationContext context,
                                DefragmentationStats* pStats = {}) const {
            vmaEndDefragmentation(
                handle, std::bit_cast<VmaDefragmentationContext>(context),
                pStats);
        }
        Result
        beginDefragmentationPass(DefragmentationContext context,
                                 DefragmentationPassMoveInfo* pPassInfo) const {
            return Result(vmaBeginDefragmentationPass(
                handle, std::bit_cast<VmaDefragmentationContext>(context),
                pPassInfo));
        }
        Result
        endDefragmentationPass(DefragmentationContext context,
                               DefragmentationPassMoveInfo* pPassInfo) const {
            return Result(vmaEndDefragmentationPass(
                handle, std::bit_cast<VmaDefragmentationContext>(context),
                pPassInfo));
        }
        Result bindBufferMemory(Buffer buffer, Allocation allocation,
                                VkDeviceSize allocationLocalOffset = 0) const {
            return Result(vmaBindBufferMemory2(
                handle, std::bit_cast<VmaAllocation>(allocation),
                allocationLocalOffset, std::bit_cast<VkBuffer>(buffer),
                nullptr));
        }
        Result bindImageMemory(Image image, Allocation allocation,
                               VkDeviceSize allocationLocalOffset = 0) const {
            return Result(vmaBindImageMemory2(
                handle, std::bit_cast<VmaAllocation>(allocation),
                allocationLocalOffset, std::bit_cast<VkImage>(image), nullptr));
        }
        Ret<Buffer>
        createBuffer(const BufferCreateInfo& bufferCreateInfo,
                     const AllocationCreateInfo& allocationCreateInfo,
                     Allocation* pAllocation,
                     AllocationInfo* pAllocationInfo = {}) const {
            Buffer value;
            return {Result(vmaCreateBuffer(
                        handle, &bufferCreateInfo, &allocationCreateInfo,
                        std::bit_cast<VkBuffer*>(&value),
                        std::bit_cast<VmaAllocation*>(pAllocation),
                        pAllocationInfo)),
                    value};
        }
        Ret<Buffer> createBufferWithAlignment(
            const BufferCreateInfo& bufferCreateInfo,
            const AllocationCreateInfo& allocationCreateInfo,
            VkDeviceSize minAlignment, Allocation* pAllocation,
            AllocationInfo* pAllocationInfo = {}) const {
            Buffer value;
            return {Result(vmaCreateBufferWithAlignment(
                        handle, &bufferCreateInfo, &allocationCreateInfo,
                        minAlignment, std::bit_cast<VkBuffer*>(&value),
                        std::bit_cast<VmaAllocation*>(pAllocation),
                        pAllocationInfo)),
                    value};
        }
        Ret<Buffer>
        createAliasingBuffer(Allocation allocation,
                             const BufferCreateInfo& bufferCreateInfo,
                             VkDeviceSize allocationLocalOffset = 0) const {
            Buffer value;
            return {Result(vmaCreateAliasingBuffer2(
                        handle, std::bit_cast<VmaAllocation>(allocation),
                        allocationLocalOffset, &bufferCreateInfo,
                        std::bit_cast<VkBuffer*>(&value))),
                    value};
        }
        void destroyBuffer(Buffer buffer, Allocation allocation) const {
            vmaDestroyBuffer(handle, std::bit_cast<VkBuffer>(buffer),
                             std::bit_cast<VmaAllocation>(allocation));
        }
        Ret<Image> createImage(const ImageCreateInfo& imageCreateInfo,
                               const AllocationCreateInfo& allocationCreateInfo,
                               Allocation* pAllocation,
                               AllocationInfo* pAllocationInfo = {}) const {
            Image value;
            return {Result(vmaCreateImage(
                        handle, &imageCreateInfo, &allocationCreateInfo,
                        std::bit_cast<VkImage*>(&value),
                        std::bit_cast<VmaAllocation*>(pAllocation),
                        pAllocationInfo)),
                    value};
        }
        Ret<Image>
        createAliasingImage(Allocation allocation,
                            const ImageCreateInfo& imageCreateInfo,
                            VkDeviceSize allocationLocalOffset = 0) const {
            Image value;
            return {Result(vmaCreateAliasingImage2(
                        handle, std::bit_cast<VmaAllocation>(allocation),
                        allocationLocalOffset, &imageCreateInfo,
                        std::bit_cast<VkImage*>(&value))),
                    value};
        }
        void destroyImage(Image image, Allocation allocation) const {
            vmaDestroyImage(handle, std::bit_cast<VkImage>(image),
                            std::bit_cast<VmaAllocation>(allocation));
        }
#if VMA_STATS_STRING_ENABLED
        char* buildStatsString(VkBool32 detailedMap) const {
            char* value;
            vmaBuildStatsString(handle, &value, detailedMap);
            return value;
        }
        void freeStatsString(char* pStatsString) const {
            vmaFreeStatsString(handle, pStatsString);
        }
#endif
    };

    struct VirtualAllocation : Handle<VmaVirtualAllocation> {};

    struct VirtualBlock : Handle<VmaVirtualBlock> {
        void destroy() const { vmaDestroyVirtualBlock(handle); }
        VkBool32 isEmpty() const { return vmaIsVirtualBlockEmpty(handle); }
        void getVirtualAllocationInfo(
            VirtualAllocation allocation,
            VirtualAllocationInfo* pVirtualAllocInfo) const {
            vmaGetVirtualAllocationInfo(
                handle, std::bit_cast<VmaVirtualAllocation>(allocation),
                pVirtualAllocInfo);
        }
        Ret<VirtualAllocation>
        virtualAllocate(const VirtualAllocationCreateInfo& createInfo,
                        DeviceSize* pOffset = {}) const {
            VirtualAllocation value;
            return {Result(vmaVirtualAllocate(
                        handle, &createInfo,
                        std::bit_cast<VmaVirtualAllocation*>(&value), pOffset)),
                    value};
        }
        void virtualFree(VirtualAllocation allocation) const {
            vmaVirtualFree(handle,
                           std::bit_cast<VmaVirtualAllocation>(allocation));
        }
        void clear() const { vmaClearVirtualBlock(handle); }
        void setVirtualAllocationUserData(VirtualAllocation allocation,
                                          void* pUserData) const {
            vmaSetVirtualAllocationUserData(
                handle, std::bit_cast<VmaVirtualAllocation>(allocation),
                pUserData);
        }
        void getStatistics(Statistics* pStats) const {
            vmaGetVirtualBlockStatistics(handle, pStats);
        }
        void calculateStatistics(DetailedStatistics* pStats) const {
            vmaCalculateVirtualBlockStatistics(handle, pStats);
        }
#if VMA_STATS_STRING_ENABLED
        char* buildStatsString(VkBool32 detailedMap) const {
            char* value;
            vmaBuildVirtualBlockStatsString(handle, &value, detailedMap);
            return value;
        }
        void freeStatsString(char* pStatsString) const {
            vmaFreeVirtualBlockStatsString(handle, pStatsString);
        }
#endif
    };

    inline Ret<Allocator>
    createAllocator(const AllocatorCreateInfo& createInfo) {
        Allocator value;
        return {Result(vmaCreateAllocator(
                    &createInfo, std::bit_cast<VmaAllocator*>(&value))),
                value};
    }

    inline Ret<VirtualBlock>
    createVirtualBlock(const VirtualBlockCreateInfo& createInfo) {
        VirtualBlock value;
        return {Result(vmaCreateVirtualBlock(
                    &createInfo, std::bit_cast<VmaVirtualBlock*>(&value))),
                value};
    }
} // namespace vklite::vma

#endif // VKLITE_VMA_HPP