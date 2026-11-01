# Shader precompilation test

## Goal

Add a per-game shader preparation screen over the guest boot. The screen must show real
progress and must not claim that every possible shader has been compiled: only
states present in the embedded seed dictionary and the game's learned seed can be
prepared during startup. The guest must boot behind the screen because its game tag
and shader seed are only known during SEGABOOT.

## Existing backend pieces

- OpenGL has per-title shader module caches and a boot/idle prewarm path
  (`module_prewarm_scan` / `module_prewarm_pump`) that loads cached program
  binaries in small time budgets.
- Vulkan reads embedded and local shader seeds in `vk_seed_siblings()`, queues
  canonical and specialised states to the `vk_spec_worker_fn` worker, and writes
  generated SPIR-V into its disk cache.
- The Vulkan SPIR-V cache is not the same as a Vulkan `VkPipelineCache`.
  Preparing SPIR-V alone must not be displayed as completed pipeline preparation.

## First implementation milestone

1. Add a backend-neutral precompile job/status API with title ID, backend,
   queued count, completed count, failed count, and an explicit
   `known_shaders_only` completion state.
2. Expose the OpenGL and Vulkan seed/prewarm queues through that API. Count only
   work actually accepted by a queue; do not estimate totals from cache file
   counts.
3. Add a modal XUI loading screen when the user chooses **Play** with shader
   caching and seeding enabled. The guest boots behind the modal; show title,
   backend, completed/total and a **Start game now** bypass action.
4. Keep compilation at low priority and bounded batches so the FX-8370E remains
   responsive. Cancellation must stop scheduling new work safely; an in-flight
   compiler call may finish.
5. Store caches separately by backend and invalidate driver-dependent pipeline
   data when the GPU driver or renderer compatibility key changes.

## Important correctness constraints

- Do not try to enumerate arbitrary game states without executing the game.
  A first-run seed can be incomplete.
- Do not block the emulation render thread while compiling.
- Do not report 100% as “all game shaders”; report “known shaders prepared”.
- OpenGL program binaries and Vulkan SPIR-V/pipeline caches are different
  artifacts and must not be conflated.
- A cache hit is not necessarily a ready GPU pipeline. Vulkan pipeline creation
  must be tracked separately from SPIR-V compilation.

## Acceptance tests

- A title with no seed displays an empty/first-run state and can still launch.
- A title with seeds displays real queued/completed counts.
- Cancel returns to the normal launch flow without corrupting cache files.
- Start now bypasses waiting but leaves safe background learning enabled.
- Repeated launch reuses the disk cache and reduces preparation work.
- OpenGL and Vulkan status and cache sizes are reported independently.
