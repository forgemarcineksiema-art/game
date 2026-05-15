# Art Direction

Last updated: 2026-05-15

This document gives Tidebreak a practical visual target before real assets exist. It is not a final art bible and does not authorize final art, model loading, textures, or a renderer overhaul in v0.11.

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

In v0.12/v0.12.1 this remains documentation plus tiny static mesh placeholders only. Do not add lighting, shadows, post-processing, or weather systems yet.

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

It does not yet mean final textures, hero assets, cinematic lighting, or detailed meshes.

## Debug-Only For Now

These remain debug/prototype-only:

- wire outlines,
- solid debug boxes,
- flat-tinted unit-box mesh placeholders,
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
