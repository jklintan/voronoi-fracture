# Voronoi Fracture

## Command Flags

The `voronoiFracture` command has the following optional flags:

| Flag             | Short Flag | Type     | Default | Explanation                                                     |
|------------------|------------|----------|---------|-----------------------------------------------------------------|
| `-num_fragments` | `-nf`      | Unsigned | 5       | Number of seed-points/voronoi-cells.                            |
| `-delete_object` | `-do`      | Boolean  | True    | Delete input object after fracture.                             |
| `-curve_radius`  | `-cr`      | Double   | 0.1     | Radius of seed-point distribution along curve.                  |
| `-disk_axis`     | `-da`      | String   | ""      | Axis of implicit sphere to use as disk-normal, "x", "y" or "z". |
| `-disk_steps`    | `-ds`      | Unsigned | 0       | Discrete radius levels to generate seed-points on.              |
| `-step_noise`    | `-sn`      | Double   | 0.05    | Sigma of noise displacement when using disk steps.              |