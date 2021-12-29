# embed project

```cmake
add_subdirectory(embed)

embed(embedded
    AS_TEXT
        ../data/shader_vertex.glsl
    AS_BINARY
        ../data/container_diffuse.png
)
add_executable(example ${other_src} ${embedded})
target_link_library(example PRIVATE embed)
```

```cpp
#include "embed/types.h"
void load_image_from_embedded(const embedded_binary&);
void load_shader(std::string_view);

#include "shader_vertex.glsl.h"
void load_shader(SHADER_VERTEX_GLSL);

#include "container_diffuse.png.h"
load_image_from_embedded(CONTAINER_DIFFUSE_PNG);
```
