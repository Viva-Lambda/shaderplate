#version 430

/*
 * Taken from supplemental code of paper
 * */
#define HIGH_RES_OCT_SPACE_MARCH 10
#define HIGH_RES_OCT_SPACE_DDA 11
#define HIGH_RES_TRACE_MODE HIGH_RES_OCT_SPACE_DDA

// Keep in sync with LightFieldModel::TraceMode
//
#define WORLD_SPACE_MARCH 0
#define OCT_SPACE_HIGH_RES_ONLY 1
#define OCT_SPACE_MULTIRES 2

// for debug
int counter = 0;

#define HANDOFF_TRACE_ALL_PROBES 3

#define MULTI_PROBE_STRATEGY HANDOFF_TRACE_ALL_PROBES

// The "thickness" of a surface depends on how much we trust the probe to
// represent visibility from
// the ray's viewpoint.  We model this as
// p = probe direction to point
// v = ray direction
// n = surface normal at point
// If the probe's viewpoint is close to the ray's,
// we're extruding away from the viewer, so thick is safe.
// Otherwise, we're extruding parallel to the ray, making objects
// artificially thick in screen space, which is not safe
//                               |
//                               v
// thickness = minThickness + (maxThickness - minThickness) * (1 - abs(dot(p,
// n)) * max(dot(p, v), 0)
// If the probe
// sees the surface at a glancing angle, then the surface potentially has a
// large depth extent in the pixel,
