#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"
#include "raymath.h"
#include "p2Point.h"

#include <math.h>

#define GRAVITY_STEP 1.0f
#define GRAVITY_MIN 1.0f
#define GRAVITY_MAX 30.0f

#define MATERIAL_STEP 0.1f // restitution and friction change in steps of 0.1, between 0 and 1

// Target FPS the player can choose in debug mode (keys 1 / 2)
static const int fps_options[] = { 30, 60, 90, 120, 144 };
static const int fps_options_count = sizeof(fps_options) / sizeof(int);

ModulePhysics::ModulePhysics(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	world = NULL;
	ground = NULL;
	mouse_joint = NULL;
	debug = false;
	accumulator = 0.0f; 
	fps_index = 1; // 60 FPS
	gravity = -GRAVITY_Y; // 15
	ball_restitution = 0.2f;
	ball_friction = 0.5f;
}

// Destructor
ModulePhysics::~ModulePhysics()
{
}

bool ModulePhysics::Start()
{
	LOG("Creating Physics 2D environment");

	SetTargetFPS(fps_options[fps_index]);

	// Assign the member variable (do NOT declare a local "b2World* world" here, it would shadow the member)
	world = new b2World(b2Vec2(GRAVITY_X, gravity));

	world->SetContactListener(this);

	// Needed to create joints like the mouse joint
	b2BodyDef bd;
	ground = world->CreateBody(&bd);

	return true;
}

update_status ModulePhysics::PreUpdate()
{
	//
	const float fixed_dt = 1.0f / 60.0f; 

	float frame_time = GetFrameTime();

	if (frame_time > 0.25f) {
		frame_time = 0.25f; 
	}
	
	accumulator += frame_time; 

	while (accumulator >= fixed_dt) {
		world->Step(1.0f / 60.0f, 8, 3);
		accumulator -= fixed_dt;
	}

	return UPDATE_CONTINUE;
}

update_status ModulePhysics::PostUpdate()
{
	if (IsKeyPressed(KEY_F1))
	{
		debug = !debug;
	}

	if (!debug)
	{
		// The mouse joint only lives in debug mode
		if (mouse_joint != NULL)
		{
			world->DestroyJoint(mouse_joint);
			mouse_joint = NULL;
		}
		return UPDATE_CONTINUE;
	}

	HandleDebugKeys();

	b2Vec2 mouse_position(PIXEL_TO_METERS(GetMouseX()), PIXEL_TO_METERS(GetMouseY()));
	b2Body* body_clicked = NULL;

	// Bonus code: this will iterate all objects in the world and draw the circles
	for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
	{
		for(b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
		{
			switch(f->GetType())
			{
				// Draw circles ------------------------------------------------
				case b2Shape::e_circle:
				{
					b2CircleShape* shape = (b2CircleShape*)f->GetShape();
					b2Vec2 pos = f->GetBody()->GetPosition();
					
					DrawCircle(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), (float)METERS_TO_PIXELS(shape->m_radius), Color{0, 0, 0, 128});

					// Radius line to see the rotation
					b2Vec2 edge = b->GetWorldPoint(b2Vec2(shape->m_radius, 0.0f));
					DrawLine(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), METERS_TO_PIXELS(edge.x), METERS_TO_PIXELS(edge.y), WHITE);
				}
				break;

				// Draw polygons ------------------------------------------------
				case b2Shape::e_polygon:
				{
					b2PolygonShape* polygonShape = (b2PolygonShape*)f->GetShape();
					int32 count = polygonShape->m_count;
					b2Vec2 prev, v;

					for(int32 i = 0; i < count; ++i)
					{
						v = b->GetWorldPoint(polygonShape->m_vertices[i]);
						if(i > 0)
							DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), RED);

						prev = v;
					}

					v = b->GetWorldPoint(polygonShape->m_vertices[0]);
					DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), RED);
				}
				break;

				// Draw chains contour -------------------------------------------
				case b2Shape::e_chain:
				{
					b2ChainShape* shape = (b2ChainShape*)f->GetShape();
					b2Vec2 prev, v;

					for(int32 i = 0; i < shape->m_count; ++i)
					{
						v = b->GetWorldPoint(shape->m_vertices[i]);
						if(i > 0)
							DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), GREEN);
						prev = v;
					}

					v = b->GetWorldPoint(shape->m_vertices[0]);
					DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), GREEN);
				}
				break;

				// Draw a single segment(edge) ----------------------------------
				case b2Shape::e_edge:
				{
					b2EdgeShape* shape = (b2EdgeShape*)f->GetShape();
					b2Vec2 v1, v2;

					v1 = b->GetWorldPoint(shape->m_vertex1);
					v2 = b->GetWorldPoint(shape->m_vertex2);
					DrawLine(METERS_TO_PIXELS(v1.x), METERS_TO_PIXELS(v1.y), METERS_TO_PIXELS(v2.x), METERS_TO_PIXELS(v2.y), BLUE);
				}
				break;
			}

			// TODO 1: If mouse button 1 is pressed, test if the current body contains mouse position
			// Only dynamic bodies can be dragged
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouse_joint == NULL && body_clicked == NULL
				&& b->GetType() == b2_dynamicBody && f->TestPoint(mouse_position))
			{
				body_clicked = b;
			}
		}
	}

	// TODO 2: If a body was selected, create a mouse joint
	// using mouse_joint class property
	if (body_clicked != NULL)
	{
		b2MouseJointDef def;
		def.bodyA = ground;
		def.bodyB = body_clicked;
		def.target = mouse_position;
		def.maxForce = 100.0f * body_clicked->GetMass();
		b2LinearStiffness(def.stiffness, def.damping, 5.0f, 0.7f, def.bodyA, def.bodyB); // 5 Hz, damping ratio 0.7

		mouse_joint = (b2MouseJoint*)world->CreateJoint(&def);
		body_clicked->SetAwake(true);
	}

	// TODO 3: If the player keeps pressing the mouse button, update
	// target position and draw a red line between both anchor points
	if (mouse_joint != NULL && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		mouse_joint->SetTarget(mouse_position);

		b2Vec2 anchor_a = mouse_joint->GetAnchorA(); // the target
		b2Vec2 anchor_b = mouse_joint->GetAnchorB(); // the point of the body we clicked
		DrawLine(METERS_TO_PIXELS(anchor_a.x), METERS_TO_PIXELS(anchor_a.y), METERS_TO_PIXELS(anchor_b.x), METERS_TO_PIXELS(anchor_b.y), RED);
	}

	// TODO 4: If the player releases the mouse button, destroy the joint
	if (mouse_joint != NULL && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
	{
		world->DestroyJoint(mouse_joint);
		mouse_joint = NULL;
	}

	DrawDebugPanel();

	return UPDATE_CONTINUE;
}


// Called before quitting
bool ModulePhysics::CleanUp()
{
	LOG("Destroying physics world");

	// Delete the whole physics world! (this also frees every body, fixture and joint it owns)
	delete world;
	world = NULL;
	ground = NULL;
	mouse_joint = NULL;

	return true;
}

// PhysBody pointer as UserData: set it in the b2BodyDef BEFORE calling CreateBody()
PhysBody* ModulePhysics::CreateCircle(int x, int y, int radius, bool dynamic)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = dynamic ? b2_dynamicBody : b2_staticBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2CircleShape shape;
	shape.m_radius = PIXEL_TO_METERS(radius);

	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 1.0f;
	b->CreateFixture(&fixture);

	pbody->body = b;

	return pbody;
}

PhysBody* ModulePhysics::CreateRectangle(int x, int y, int half_width, int half_height, bool dynamic)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = dynamic ? b2_dynamicBody : b2_staticBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(half_width), PIXEL_TO_METERS(half_height));

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	b->CreateFixture(&fixture);

	pbody->body = b;

	return pbody;
}

// Static box that detects overlaps but does not collide (a trigger)
PhysBody* ModulePhysics::CreateRectangleSensor(int x, int y, int half_width, int half_height)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_staticBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(half_width), PIXEL_TO_METERS(half_height));

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.isSensor = true;
	b->CreateFixture(&fixture);

	pbody->body = b;

	return pbody;
}

// size is the number of ints, not vertices
PhysBody* ModulePhysics::CreateChain(int x, int y, const int* points, int size)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	int count = size / 2;
	b2Vec2* vertices = new b2Vec2[count];
	for (int i = 0; i < count; ++i)
	{
		vertices[i].x = PIXEL_TO_METERS(points[i * 2 + 0]);
		vertices[i].y = PIXEL_TO_METERS(points[i * 2 + 1]);
	}

	b2ChainShape shape;
	shape.CreateLoop(vertices, count);
	delete[] vertices; // CreateLoop copies the vertices

	b2FixtureDef fixture;
	fixture.shape = &shape;
	b->CreateFixture(&fixture);

	pbody->body = b;

	return pbody;
}

// Dynamic polygon (points relative to the hinge) attached to the ground at (x, y) with a motorized revolute joint
PhysBody* ModulePhysics::CreateFlipper(int x, int y, const int* points, int size, float lower_angle, float upper_angle, float start_angle, b2RevoluteJoint** joint)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.angle = start_angle * DEGTORAD;
	body.bullet = true;
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	// Box2D polygons: convex and max 8 vertices
	int count = size / 2;
	if (count > b2_maxPolygonVertices)
		count = b2_maxPolygonVertices;

	b2Vec2 vertices[b2_maxPolygonVertices];
	for (int i = 0; i < count; ++i)
		vertices[i].Set(PIXEL_TO_METERS(points[i * 2 + 0]), PIXEL_TO_METERS(points[i * 2 + 1]));

	b2PolygonShape shape;
	shape.Set(vertices, count);

	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 1.0f;
	b->CreateFixture(&fixture);

	// Hinge: the flipper can only rotate around (x, y), between the two limits, pushed by the motor
	b2RevoluteJointDef def;
	def.bodyA = ground;
	def.bodyB = b;
	def.localAnchorA.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y)); // hinge point in the world
	def.localAnchorB.Set(0.0f, 0.0f);                               // hinge point in the flipper (its origin)
	def.enableLimit = true;
	def.lowerAngle = lower_angle * DEGTORAD;
	def.upperAngle = upper_angle * DEGTORAD;
	def.enableMotor = true;
	def.maxMotorTorque = 200.0f;
	def.motorSpeed = 0.0f;

	*joint = (b2RevoluteJoint*)world->CreateJoint(&def);

	pbody->body = b;

	return pbody;
}

// Dynamic box that can only slide vertically (prismatic joint), from (x, y) down to (x, y + travel)
PhysBody* ModulePhysics::CreateKicker(int x, int y, int half_width, int half_height, int travel, b2PrismaticJoint** joint)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.fixedRotation = true; // the plunger never rotates
	body.bullet = true;
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(half_width), PIXEL_TO_METERS(half_height));

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	b->CreateFixture(&fixture);

	// Vertical rail: axis (0, 1) points DOWN on screen, so a positive translation = plunger pulled down
	b2PrismaticJointDef def;
	def.Initialize(ground, b, b->GetPosition(), b2Vec2(0.0f, 1.0f));
	def.enableLimit = true;
	def.lowerTranslation = 0.0f;                     // rest (top)
	def.upperTranslation = PIXEL_TO_METERS(travel);  // fully charged (bottom)
	def.enableMotor = true;
	def.maxMotorForce = 200.0f;
	def.motorSpeed = 0.0f;

	*joint = (b2PrismaticJoint*)world->CreateJoint(&def);

	pbody->body = b;

	return pbody;
}

void ModulePhysics::DestroyBody(PhysBody* pbody)
{
	if (pbody == NULL)
		return;

	// Box2D also destroys the joints of this body: forget the mouse joint
	if (mouse_joint != NULL && mouse_joint->GetBodyB() == pbody->body)
		mouse_joint = NULL;

	world->DestroyBody(pbody->body); // Box2D frees the body and its fixtures
	delete pbody;                    // we free our own wrapper
}

// Call the listeners that are not NULL
void ModulePhysics::BeginContact(b2Contact* contact)
{
	PhysBody* a = (PhysBody*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
	PhysBody* b = (PhysBody*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

	if (a != NULL && a->listener != NULL)
		a->listener->OnCollision(a, b);

	if (b != NULL && b->listener != NULL)
		b->listener->OnCollision(b, a);
}

// PhysBody ------------------------------------
void PhysBody::GetPosition(int& x, int& y) const
{
	b2Vec2 pos = body->GetPosition();
	x = METERS_TO_PIXELS(pos.x);
	y = METERS_TO_PIXELS(pos.y);
}

float PhysBody::GetRotation() const
{
	return body->GetAngle();
}

bool PhysBody::Contains(int x, int y) const
{
	b2Vec2 p(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	// True if the point is inside ANY of the shapes of this body
	for (b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext())
	{
		if (fixture->GetShape()->TestPoint(body->GetTransform(), p))
			return true;
	}

	return false;
}

int PhysBody::RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const
{
	// No hit: return -1
	// Hit: fill normal_x and normal_y and return the distance between x1,y1 and the hit point
	int ret = -1;

	b2RayCastInput input;
	input.p1.Set(PIXEL_TO_METERS(x1), PIXEL_TO_METERS(y1));
	input.p2.Set(PIXEL_TO_METERS(x2), PIXEL_TO_METERS(y2));
	input.maxFraction = 1.0f;

	float closest = 1.0f;

	for (b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext())
	{
		// A chain has one child per edge
		for (int32 i = 0; i < fixture->GetShape()->GetChildCount(); ++i)
		{
			b2RayCastOutput output;
			if (fixture->GetShape()->RayCast(&output, input, body->GetTransform(), i) && output.fraction <= closest)
			{
				closest = output.fraction;

				float dx = (float)(x2 - x1);
				float dy = (float)(y2 - y1);
				ret = (int)(output.fraction * sqrtf(dx * dx + dy * dy));

				normal_x = output.normal.x;
				normal_y = output.normal.y;
			}
		}
	}

	return ret;
}

// Keys that change the simulation (only in debug mode)
void ModulePhysics::HandleDebugKeys()
{
	// Target FPS: 1 lower, 2 higher
	if (IsKeyPressed(KEY_ONE) && fps_index > 0)
		SetTargetFPS(fps_options[--fps_index]);
	if (IsKeyPressed(KEY_TWO) && fps_index < fps_options_count - 1)
		SetTargetFPS(fps_options[++fps_index]);

	// Gravity: 3 lower, 4 higher
	bool gravity_changed = false;
	if (IsKeyPressed(KEY_THREE) && gravity > GRAVITY_MIN)
	{
		gravity -= GRAVITY_STEP;
		gravity_changed = true;
	}
	if (IsKeyPressed(KEY_FOUR) && gravity < GRAVITY_MAX)
	{
		gravity += GRAVITY_STEP;
		gravity_changed = true;
	}

	if (gravity_changed)
	{
		world->SetGravity(b2Vec2(GRAVITY_X, gravity));

		// Bodies at rest are "asleep" and would ignore the new gravity until something touches them
		for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
			b->SetAwake(true);
	}

	// Ball restitution: 5 lower, 6 higher. Ball friction: 7 lower, 8 higher
	bool material_changed = false;
	if (IsKeyPressed(KEY_FIVE)) { ball_restitution -= MATERIAL_STEP; material_changed = true; }
	if (IsKeyPressed(KEY_SIX)) { ball_restitution += MATERIAL_STEP; material_changed = true; }
	if (IsKeyPressed(KEY_SEVEN)) { ball_friction -= MATERIAL_STEP; material_changed = true; }
	if (IsKeyPressed(KEY_EIGHT)) { ball_friction += MATERIAL_STEP; material_changed = true; }

	if (material_changed)
	{
		// Keep them in [0, 1] and round to 1 decimal (0.1 is not exact in float: avoids 0.30000001)
		ball_restitution = roundf(Clamp(ball_restitution, 0.0f, 1.0f) * 10.0f) / 10.0f;
		ball_friction = roundf(Clamp(ball_friction, 0.0f, 1.0f) * 10.0f) / 10.0f;
		ApplyBallMaterial();
	}
}

// Small panel with the current values and the keys to change them
void ModulePhysics::DrawDebugPanel() const
{
	DrawRectangle(10, 48, 190, 70, Fade(BLACK, 0.75f));
	DrawText(TextFormat("FPS %d / %d   [1] [2]", GetFPS(), fps_options[fps_index]), 18, 55, 10, GREEN);
	DrawText(TextFormat("Gravity %.0f m/s2   [3] [4]", gravity), 18, 69, 10, GREEN);
	DrawText(TextFormat("Ball bounce %.1f   [5] [6]", ball_restitution), 18, 83, 10, GREEN);
	DrawText(TextFormat("Ball friction %.1f   [7] [8]", ball_friction), 18, 97, 10, GREEN);
}

// Applies the current restitution and friction to every ball on the table
void ModulePhysics::ApplyBallMaterial()
{
	for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
	{
		b2Fixture* fixture = b->GetFixtureList();

		// The balls are the only dynamic bodies with a circle shape
		if (b->GetType() == b2_dynamicBody && fixture != NULL && fixture->GetType() == b2Shape::e_circle)
		{
			fixture->SetRestitution(ball_restitution);
			fixture->SetFriction(ball_friction);
		}
	}
}
