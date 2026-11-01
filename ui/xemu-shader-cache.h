/*
 * xemu shader cache bookkeeping per game
 *
 * The renderers keep compiled shaders on disk for all games together. To
 * clear one game's shaders, the files each game creates or uses are noted
 * here while it runs and listed in <base>/shader_cache_titles/<ID>.txt.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef XEMU_SHADER_CACHE_H
#define XEMU_SHADER_CACHE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    XEMU_SHADER_BACKEND_OPENGL = 1,
    XEMU_SHADER_BACKEND_VULKAN = 2,
};

typedef struct XemuShaderPrecompileStatus {
    uint32_t title_id;
    uint32_t gl_done, gl_total;
    uint32_t vk_done, vk_total;
    bool gl_started, gl_finished;
    bool vk_started, vk_finished;
} XemuShaderPrecompileStatus;

/* Progress counts only work discovered in the game's known shader seeds. */
void xemu_shader_precompile_reset(uint32_t title_id);
void xemu_shader_precompile_update(int backend, uint32_t done,
                                   uint32_t total, bool finished,
                                   bool available);
void xemu_shader_precompile_get(XemuShaderPrecompileStatus *status);

/* A shader cache file was written or read for the running game (any
 * thread) */
void xemu_shader_cache_note(const char *path);

/* After a clear: don't write the Vulkan pipeline cache again this session,
 * so what was in memory doesn't come back */
bool xemu_shader_cache_pipelines_discarded(void);

#ifdef __cplusplus
}
#endif

#endif
