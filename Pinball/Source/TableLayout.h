#pragma once

// ============================================================================
// TABLE LAYOUT: where everything is on the table (window 514x855, in pixels)
// Only positions and shapes here: the rules (points, forces...) live in each entity
// Chains are one-sided: outer wall COUNTER-CLOCKWISE (ball inside), obstacles CLOCKWISE (ball outside)
// ============================================================================

// --- Walls (chains) ---------------------------------------------------------

// Outer wall + kicker lane (the floor of the drain is below the screen, y 900)
static const int table_outer[] = {
	312, 900, 312, 828, 314, 824, 455, 760, 460, 755, 463, 748, 463, 595, 419, 543,
	417, 536, 417, 415, 437, 353, 458, 300, 445, 247, 394, 358, 388, 359, 383, 357,
	382, 351, 441, 228, 417, 129, 388, 128, 385, 124, 386, 88, 398, 88, 423, 113,
	440, 137, 455, 163, 464, 185, 467, 205, 469, 230, 469, 840, 501, 840, 500, 238,
	498, 194, 494, 176, 489, 158, 479, 137, 467, 116, 453, 97, 437, 79, 423, 64,
	405, 49, 387, 37, 366, 27, 346, 21, 323, 15, 300, 11, 275, 10, 250, 10,
	221, 12, 197, 17, 176, 24, 156, 32, 140, 40, 124, 50, 108, 62, 91, 76,
	78, 88, 62, 104, 50, 119, 39, 136, 30, 156, 22, 181, 16, 204, 13, 227,
	10, 531, 43, 566, 43, 573, 10, 606, 8, 610, 8, 757, 12, 763, 145, 826,
	148, 831, 148, 900
};

static const int left_rail[] = {
	72, 216, 66, 218, 59, 216, 54, 212, 52, 206, 55, 195, 67, 176, 84, 149,
	95, 134, 108, 116, 124, 101, 140, 89, 161, 77, 182, 68, 194, 64, 201, 64,
	206, 68, 209, 76, 208, 83, 205, 88, 199, 93, 188, 95, 178, 99, 166, 106,
	154, 114, 141, 124, 127, 137, 116, 149, 105, 164, 86, 192, 76, 208
};

static const int inverted_v[] = {
	164, 154, 168, 159, 171, 166, 171, 262, 174, 270, 199, 298, 218, 324, 228, 343,
	232, 356, 233, 374, 232, 388, 226, 389, 223, 385, 223, 370, 213, 349, 193, 319,
	169, 292, 142, 268, 133, 266, 124, 269, 106, 289, 96, 311, 88, 337, 83, 370,
	79, 373, 74, 370, 67, 359, 62, 345, 63, 335, 72, 296, 76, 284, 154, 158,
	159, 154
};

// Triangle under the inverted V
static const int triangle[] = {
	108, 385, 108, 379, 125, 340, 130, 334, 137, 332,
	145, 334, 186, 378, 188, 383, 185, 387, 111, 387
};

static const int left_flipper_guide[] = {
	37, 640, 40, 636, 45, 637, 47, 642, 44, 645, 45, 720, 50, 723,
	148, 762, 154, 768, 155, 776, 151, 781, 144, 781, 38, 729, 39, 644
};

static const int right_flipper_guide[] = {
	320, 763, 422, 721, 424, 646, 421, 641, 424, 636, 429, 637, 431, 642,
	429, 646, 430, 729, 325, 780, 319, 780, 313, 776, 313, 769
};

// --- Bumpers ----------------------------------------------------------------

// Center of the top face (same order as the textures: pink, blue, orange)
static const int bumper_pos[3][2] = { { 350, 202 }, { 240, 203 }, { 303, 310 } };

// --- Slingshots -------------------------------------------------------------

// Pink face of the sprite (chain)
static const int slingshot_left_shape[] = {
	95, 592, 99, 594, 106, 605, 144, 687, 144, 693, 137, 699, 99, 686, 95, 682
};
static const int slingshot_right_shape[] = {
	370, 592, 370, 682, 366, 686, 328, 699, 321, 693, 321, 687, 360, 603, 366, 594
};

// Top-left of the sprite
static const int slingshot_left_sprite[2] = { 92, 592 };
static const int slingshot_right_sprite[2] = { 321, 592 };

// Rubber line a -> b (ax, ay, bx, by), ordered so its outer side is on the LEFT of a -> b
static const int slingshot_left_rubber[4] = { 106, 605, 144, 687 };
static const int slingshot_right_rubber[4] = { 321, 687, 360, 603 };

// --- Side light targets -----------------------------------------------------

// Top-left of each sprite (top to bottom: orange, blue, pink). The box is 7x34, just in front of the wall
static const int target_left_x = 13;
static const int target_right_x = 408;
static const int target_left_y[3] = { 420, 457, 494 };
static const int target_right_y[3] = { 429, 466, 503 };

// --- Crystals ---------------------------------------------------------------

// Center: top of the dome, right diagonal slot, inside the arch of the V
static const int crystal_pos[3][2] = { { 292, 94 }, { 442, 290 }, { 133, 302 } };

// --- Flippers ---------------------------------------------------------------

// Hinge position on the table
static const int flipper_left_hinge[2] = { 145, 770 };
static const int flipper_right_hinge[2] = { 320, 770 };

// Hinge position inside the sprite
static const int flipper_left_pivot[2] = { 8, 9 };
static const int flipper_right_pivot[2] = { 71, 9 };

// Polygons relative to the hinge (convex, max 8 points)
static const int flipper_left_shape[] = { -8, 0, -5, -7, 2, -9, 67, -4, 71, 0, 67, 5, 2, 10, -5, 7 };
static const int flipper_right_shape[] = { 8, 0, 5, 7, -2, 10, -67, 5, -71, 0, -67, -4, -2, -9, 5, -7 };

// --- Kicker and ball --------------------------------------------------------

static const int kicker_x = 485;        // center of the lane
static const int kicker_rest_y = 636;   // plunger center at rest
static const int kicker_travel = 126;   // how far down it can be pulled
static const int kicker_bottom_y = 840; // bottom of the lane (spring anchor)

static const int ball_spawn[2] = { 485, 620 }; // resting on the plunger

// --- Death zone -------------------------------------------------------------

// Sensor below the screen, between the drain walls: center x, center y, half width, half height
static const int death_zone[4] = { 230, 885, 80, 10 };

// --- Decorations (no physics) -----------------------------------------------

// Decorative arrow: center, rotation in degrees and its step in the chase (0 = first to light)
struct DecoArrow
{
	int x, y;
	float angle;
	int step;
};

// Arrows next to the slingshots, pointing to the outlanes (the sprite points up)
static const DecoArrow side_arrows[] = {
	{ 67, 624, 180.0f, 0 }, { 67, 656, 180.0f, 1 }, { 74, 692, 145.0f, 2 },   // left
	{ 400, 624, 180.0f, 0 }, { 400, 656, 180.0f, 1 }, { 393, 691, -145.0f, 2 }, // right
};

// Arrows in the middle, between the flippers, pointing up (chase from the bottom one to the top one)
static const DecoArrow center_arrows[] = {
	{ 230, 699, 0.0f, 0 }, { 230, 663, 0.0f, 1 }, { 230, 630, 0.0f, 2 }, { 230, 595, 0.0f, 3 },
};

// --- Rail lights ------------------------------------------------------------

// Center of the 5 round lights between the left rail and the V (top to bottom)
static const int rail_light_pos[5][2] = {
	{ 164, 126 }, { 143, 144 }, { 125, 168 }, { 109, 193 }, { 91, 218 }
};

// Pink arrows under the inverted V, pointing to its two lanes (they blink alternately)
static const DecoArrow v_arrows[] = {
	{ 83, 406, 18.0f, 0 }, { 208, 408, -14.0f, 1 },
};

// Right lane: up arrow + 3 triangles pointing up the diagonal (chase upwards)
static const DecoArrow right_lane_arrow[] = {
	{ 399, 399, 23.0f, 0 },
};
static const DecoArrow right_lane_triangles[] = {
	{ 411, 370, 23.0f, 1 }, { 422, 344, 23.0f, 2 }, { 432, 322, 23.0f, 3 },
};

// Pink triangles inside the kicker tube (drawn translucent, chase upwards)
static const DecoArrow tube_triangles[] = {
	{ 483, 533, 0.0f, 0 }, { 483, 497, 0.0f, 1 }, { 483, 464, 0.0f, 2 }, { 483, 424, 0.0f, 3 },
	{ 483, 387, 0.0f, 4 }, { 483, 352, 0.0f, 5 }, { 483, 312, 0.0f, 6 }, { 483, 275, 0.0f, 7 },
};