# vklite
Lightweight C++ wrapper for Vulkan

## Usage
```c++
#include <volk.h> // or other vulkan API header
#include <vklite/vulkan.hpp>
#include <vklite/vk_mem_alloc.hpp>

namespace vk = vklite;
namespace vma = vk::vma;
```

## Features
* Type safety for enums and bitfields.
* Compile-time viability checking for struct extensions (i.e. those chained with `pNext`).
* No need to specify `sType` yourself.
* Informative comments from `vk.xml` registry.
* Organized in a way such that you can lookup the related version/extension for an entity (e.g. enum/struct/function) quickly.
* Default parameters.

## Why not Vulkan-Hpp
It's too heavyweight and almost unusable without using the C++ module version.

## Naming Conventions
### Types
`VkName` -> `vk::Name`

### Free Functions 
`vkName` -> `vk::name`

### Handle Functions 
`vkHandleName(handle, ...)` -> `handle.name(...)`

### Enums
`VK_ENUM_NAME` -> `vk::Enum::eName`

### FlagBits
`VK_ENUM_NAME_BIT` -> `vk::EnumFlagBits::bName`

In all other cases the extension suffix is preserved.

## Struct
### Constructors
* `sType` is automatically set.
* Default constructor is value-initialized.
* Parametric constructor is available if all fields are required.

### Properties
* `prop` -> `setProp`/`getProp`
* `pProp` -> `setProp`/`getProp`

`setProp` is available if the struct is not _returnedonly_.

### Extensions
If a struct is extensible, `attach` member functions for each extension are available.
If the attachment must be the first one, it's named `attachHead` instead.
#### Example
```c++
vx::PipelineShaderStageCreateInfo shaderStageInfo;
vk::ShaderModuleCreateInfo moduleInfo;
vk::DebugUtilsObjectNameInfoEXT objectNameInfo;
shaderStageInfo.attachHead(moduleInfo);
shaderStageInfo.attach(objectNameInfo);
```

## Calling Conventions
* `void function(..., T* out)` -> `T function(...)`
* `VkResult function(..., T* out)` -> `vk::Ret<T> function(...)`