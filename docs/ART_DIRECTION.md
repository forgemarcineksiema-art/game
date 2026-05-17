# Art Direction

Last updated: 2026-05-17

This document gives Tidebreak a practical visual target before real assets exist. It is not a final art bible and does not authorize final art, textures, materials, lighting, or a renderer overhaul.

## Visual Target

Tidebreak should feel like a grounded coastal island sandbox: damp, practical, slightly neglected, and human-scale. The player should read spaces as useful places, not abstract obstacle courses.

The target is not photorealism in v0.x. The target is clear mood, readable silhouettes, believable scale, and a consistent material/prop language that can survive the move from debug geometry to real assets.

## Veyra Reach Mood

Veyra Reach should suggest:

- ferry routes and broken local infrastructure,
- salt air, wet timber, rusted service metal, old concrete,
- practical public buildings rather than glossy tourist spaces,
- narrow roads, dock edges, service yards, hillside access paths,
- a place that remembers repairs, shortcuts, blocked routes, and favors.

Current runtime foundation: Cinder Harbor Reach is the first authored Veyra location. It should read as a coastal/industrial remote-crime edge: Greywinch Service Road, Saltbreak Lay-by, Stormwall Bend, Old Pump Cut, Reach Relay Hut, Cracked Causeway Spur, Low Beacon Turn, Harbor Scar Overlook, and West Ferry Spur. The current art is still placeholder-grade, but it must be composed as a place: connected roads, shore/water/backdrop, industrial silhouettes, road-edge posts/barriers, cargo/cache risk, and readable overlook/relay landmarks.

The sky/horizon proof is currently a pale renderer clear color plus authored horizon, water, and backdrop geometry. Do not describe this as a real skybox, weather, lighting, or atmospheric renderer.

## Ferry Office Mood

The Ferry Office should read as:

- a small dock-side public/service building,
- a maintenance side yard with equipment and barriers,
- an office route blocked by power/service access,
- a place where a manifest, maintenance box, wall button, and service gate belong.

If a screenshot only looks like colored wireframe boxes, the scene is not yet meeting the mood target.

## Palette Direction

Prefer restrained, coastal-industrial color relationships:

- wet timber browns and muted dock yellows,
- harbor blue/green water accents,
- oxidized green service equipment,
- rusted orange/brown trim in small amounts,
- cold concrete/stone neutrals,
- pale fog/sky values later.

Avoid a one-note palette. Do not let the scene become only dark blue, only beige, or only brown/orange.

## Lighting And Weather Direction

Future rendering should aim for:

- overcast coastal light,
- soft haze/fog,
- damp surfaces,
- readable silhouettes,
- gentle contrast rather than harsh action-game lighting.

In v0.12-v0.18 this remains documentation plus tiny static mesh placeholders only. Do not add lighting, shadows, post-processing, or weather systems yet.

## Prop Language

Good first prop families:

- ferry signs and timetable boards,
- weathered railings and bollards,
- maintenance boxes and service panels,
- crates, cable reels, cones, barriers,
- simple dock planks and office counters,
- small vehicle/service-yard objects.

Every prop should answer: why is it here, what scale is it, and what does it help the player read?

## Not Placeholder-Looking Means

For this project, "not placeholder-looking" first means:

- consistent scale,
- stable composition,
- recognizable place types,
- distinct object roles,
- readable route guidance,
- coherent color keys,
- no random shapes without purpose.

For Cinder Harbor Reach specifically, a placeholder is acceptable only if it has authored identity and spatial purpose: terrain patch, road ribbon, shoreline/water/backdrop, road edge, utility/relay landmark, cargo/yard cue, or route/collision proof. Random debug boxes, isolated markers, and empty pads are not acceptable as the final result of a world-foundation goal.

It does not yet mean final textures, hero assets, cinematic lighting, or detailed meshes.

## Debug-Only For Now

These remain debug/prototype-only:

- wire outlines,
- solid debug boxes,
- flat-tinted unit-box mesh placeholders,
- flat-tinted v0.18 prop-kit mesh placeholders,
- GDI debug text,
- route polylines,
- interactable cubes/beacons,
- traversal markers,
- vehicle proxy boxes,
- collision visualization colors.

They should stay useful until a later goal replaces them with real authored assets and a debug overlay.

## Near-Term Visual Priorities

1. Keep the Ferry Office readable as a place.
2. Use the v0.12 static mesh spike to replace only a few purposeful placeholders at a time.
3. Preserve debug visibility while real assets are added.
4. Build a small original prop kit before expanding the map.
5. Keep screenshots as visual evidence after major presentation changes.

## v0.12.1 Visual Observation

The v0.12.1 screenshot at `docs/images/v0.12.1-gdi-screenshot.png` shows a clearer Ferry Office front, sign-board cue, dock bollards, service-yard crate, and vehicle cabin silhouette. The scene reads more like a dock/service-yard layout than v0.12, but it is still visibly a debug placeholder scene: wire overlays, route/debug markers, and flat-tinted unit boxes remain essential for playtesting.

## v0.14 Dock Road Observation

The v0.14 dock road pass extends the service-yard into a compact island service road with a dark road pad, turn-around/end marker, shore/water edge cue, and weathered rail/curb edges. This improves the first read of "vehicle near a dock road" without changing the renderer or adding final art. The strongest remaining visual limitation is still composition and authored assets: the area is more legible, but it remains a hand-mirrored debug layout rather than a production scene.

## v0.18 Prop Style Observation

The v0.18 pass adds the first original service-road prop-kit silhouettes: a route sign, road-edge posts, a low service barrier, and a utility cabinet. The screenshot at `docs/images/v0.18-gdi-screenshot.png` shows the current GDI reference. These are still flat-tinted, tiny-subset `.gltf` placeholders, but they shift the scene away from pure scaled boxes toward a practical coastal service-road language. The next visual risk is workflow, not quantity: before many more props are added, the project should decide whether to keep hand-authored tiny `.gltf` files briefly or stabilize a real static-mesh import path.

## v0.19 Asset Workflow Observation

v0.19 intentionally does not change the visual composition. It stabilizes the path that allowed the v0.18 props to exist: every committed mesh file should be visible to `tools/mesh_report.py`, referenced by scene data, documented with license/provenance, and accepted by `tools/validate_assets.py`. This keeps future visual identity work from becoming a pile of untracked prop experiments.

## v0.20 Blender Workflow Observation

v0.20 adds one small fallback-generated ferry notice board near the office approach. It supports the dock/ferry-service identity, but it is explicitly not a Blender export because Blender was not available in the current environment. Treat it as a workflow placeholder: useful for route readability and validation, not a proof that real DCC art is flowing yet.

## v0.20.1 Blender Export Observation

v0.20.1 adds a second notice-board style prop, this time generated through a real headless Blender 5.1.1 script. It is still low-detail flat placeholder geometry, but it proves the first DCC-authored prop path and gives the ferry-office approach a more intentional service-location cue without adding textures, materials, lighting, or final art.

## v0.27 Renderer / Cable Reel Observation

v0.27 adds painter-depth ordering for projected solid-box and flat-mesh triangle batches, plus one small Blender-authored cable reel in the service yard. This is still debug rendering, not real lighting/material/depth-buffer presentation, but it should make overlapping flat meshes feel less arbitrary while giving the service yard one more practical infrastructure silhouette.

## v0.38 Clean Presentation / Overcast Shading Observation

v0.38 makes the default playtest view less like a debug editor: raw route lines, wire boxes, trigger radii, and marker beacons are now reserved for F1/debug mode. Scene boxes and flat mesh triangles receive a small fixed overcast face-shading treatment so existing props read as volumes instead of single-color cutouts. The dock-road endpoint also reuses existing road posts, barrier, utility-box, and cable-reel meshes for a storm-wet work-zone cue. This is still not final art, textures, materials, lighting, terrain, or a production character model, but the saved screenshot at `docs/images/v0.38-gdi-playtest-clean-shaded.png` is the current visual reference for the cleaner playtest presentation.

## v0.42 Wet Road Surface Observation

v0.42 replaces the biggest flat service-yard and dock-road visual slabs with a shallow Blender-authored wet-road surface mesh. The surface is still untextured placeholder geometry, but longitudinal seams and low edge lips give the vehicle route a more intentional road/service-pad read under the current overcast material shading. The remaining risk is still asset and renderer maturity: this is not terrain, tire physics, road splines, real materials, textures, puddles, decals, or lighting.

## v0.43 Harbor Backdrop Observation

v0.43 adds a low distant-shore/harbor silhouette around the dock-road and dock-start water bands. This is the first small step from a self-contained debug yard toward a place that hints at Veyra Reach beyond the immediate route. The backdrop should stay quiet and misty: it supports the island setting without becoming a fake open-world promise, terrain system, collision boundary, city block, or final skyline.

## v0.44 Harbor Water Surface Observation

v0.44 gives the three existing water-edge bands a shallow static ripple surface mesh. This keeps the water treatment honest for the current renderer: more authored and less strip-like, but still flat-tinted, untextured, nontransparent placeholder geometry. It should support the damp coastal mood without pretending Tidebreak has water simulation, reflective shaders, shoreline terrain, foam, or weather yet.

## v0.93 Ferry Office Service Panel Observation

v0.93 replaces the office-side service/control panel's generic utility-box reuse with a small non-text panel mesh. It is still fallback placeholder geometry, but the raised frame, breaker bars, and conduit cue make the office controls read more like a practical service place while preserving the no-readable-text/no-branding rule.
