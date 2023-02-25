// AI framework
// - rlyeh, public domain.
//
// original code by https://github.com/Cultrarius/Swarmz (UNLICENSE)

vec3 rnd3() { // random uniform
    float theta = randf() * C_PI * 2;
    float r = sqrt(randf());
    float z = sqrt(1.0f - r * r) * (randf() > 0.5f ? -1.0f : 1.0f);
    return vec3(r * cos(theta), r * sin(theta), z);
}
int less3(vec3 *lhs, vec3 *rhs) {
    if(lhs->x != rhs->x) return lhs->x - rhs->x;
    if(lhs->y != rhs->y) return lhs->y - rhs->y;
    if(lhs->z != rhs->z) return lhs->z - rhs->z;
    return 0;
}
uint64_t hash3(vec3 *v) {
    uint64_t h1 = hash_flt(v->x);
    uint64_t h2 = hash_flt(v->y);
    uint64_t h3 = hash_flt(v->z);
    return (h1 * 31 + h2) * 31 + h3;
}
vec3 clamplen3(vec3 v, float length) {
    return len3(v) <= length ? v : scale3(norm3(v), length);
}

float transform_distance(float distance, SWARM_DISTANCE type) {
    float quad;
    /**/ if (type == SWARM_DISTANCE_LINEAR)            return distance;
    else if (type == SWARM_DISTANCE_QUADRATIC)         return distance * distance;
    else if (type == SWARM_DISTANCE_INVERSE_LINEAR)    return distance == 0 ? 0 : 1 / distance;
    else if (type == SWARM_DISTANCE_INVERSE_QUADRATIC) return (quad = distance * distance), (quad == 0 ? 0 : 1 / quad);
    return distance; // throw exception instead?
}

typedef struct nearby_boid_t {
    boid_t *boid;
    vec3 direction;
    float distance;
} nearby_boid_t;

static
vec3 get_voxel_for_boid(float perception_radius, const boid_t *b) { // quantize position
    float r = absf(perception_radius);
    return vec3( (int)(b->position.x / r), (int)(b->position.y / r), (int)(b->position.z / r) );
}

static
void check_voxel_for_boids(float perception_radius, float blindspot_angledeg_compare_value, array(boid_t*) voxel_cached, array(nearby_boid_t) *result, const vec3 voxelPos, const boid_t *b) {
    for each_array_ptr(voxel_cached, const boid_t*, test) {
        vec3 p1 = b->position;
        vec3 p2 = (*test)->position;
        vec3 vec = sub3(p2, p1);
        float distance = len3(vec);

        float compare_value = 0;
        float l1 = len3(vec);
        float l2 = len3(b->velocity);
        if (l1 != 0 && l2 != 0) {
            compare_value = dot3(neg3(b->velocity), vec) / (l1 * l2);
        }

        if ((&b) != test && distance <= perception_radius && (blindspot_angledeg_compare_value > compare_value || len3(b->velocity) == 0)) {
            nearby_boid_t nb;
            nb.boid = (boid_t*)*test;
            nb.distance = distance;
            nb.direction = vec;
            array_push(*result, nb);
        }
    }
}

static
array(nearby_boid_t) get_nearby_boids(const swarm_t *self, const boid_t *b) {
    array(nearby_boid_t) result = 0;
    array_reserve(result, array_count(self->boids));

    vec3 voxelPos = get_voxel_for_boid(self->perception_radius, b);
    voxelPos.x -= 1;
    voxelPos.y -= 1;
    voxelPos.z -= 1;
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            for (int z = 0; z < 3; z++) {
                array(boid_t*) *found = map_find(self->voxel_cache_, &voxelPos);
                if( found ) check_voxel_for_boids(self->perception_radius, self->blindspot_angledeg_compare_value_, *found, &result, voxelPos, b);
                voxelPos.z++;
            }
            voxelPos.z -= 3;
            voxelPos.y++;
        }
        voxelPos.y -= 3;
        voxelPos.x++;
    }
    return result;
}

static
void update_boid(swarm_t *self, boid_t *b) {
    vec3 separation_sum = {0};
    vec3 heading_sum = {0};
    vec3 position_sum = {0};
    vec3 po = b->position;

    array(nearby_boid_t) nearby = get_nearby_boids(self, b); // @leak

    for each_array_ptr(nearby, nearby_boid_t, closeboid_t) {
        if (closeboid_t->distance == 0) {
            separation_sum = add3(separation_sum, scale3(rnd3(), 1000)); //addscale3
        }
        else {
            float separation_factor = transform_distance(closeboid_t->distance, self->separation_type);
            separation_sum = add3(separation_sum, scale3(neg3(closeboid_t->direction), separation_factor)); // addscale3
        }
        heading_sum = add3(heading_sum, closeboid_t->boid->velocity); // inc3
        position_sum = add3(position_sum, closeboid_t->boid->position); // inc3
    }

    vec3 steering_target = b->position;
    float target_distance = -1;
    for( int i = 0, end = array_count(self->steering_targets); i < end; ++i ) {
        vec3 *target = &self->steering_targets[i];

        float distance = transform_distance(len3(sub3(*target,b->position)), self->steering_target_type);
        if (target_distance < 0 || distance < target_distance) {
            steering_target = *target;
            target_distance = distance;
        }
    }

    int nearby_size = array_count(nearby);

    // Separation: steer to avoid crowding local flockmates
    vec3 separation = nearby_size > 0 ? scale3(separation_sum, 1.f / nearby_size) : separation_sum;

    // Alignment: steer towards the average heading of local flockmates
    vec3 alignment = nearby_size > 0 ? scale3(heading_sum, 1.f / nearby_size) : heading_sum;

    // Cohesion: steer to move toward the average position of local flockmates
    vec3 avgposition = nearby_size > 0 ? scale3(position_sum, 1.f / nearby_size) : b->position;
    vec3 cohesion = sub3(avgposition, b->position);

    // Steering: steer towards the nearest target location (like a moth to the light)
    vec3 steering = scale3(norm3(sub3(steering_target, b->position)), target_distance);

    // calculate boid acceleration
    vec3 acceleration;
    acceleration = scale3(separation, self->separation_weight);
    acceleration = add3(acceleration, scale3(alignment, self->alignment_weight));
    acceleration = add3(acceleration, scale3(cohesion, self->cohesion_weight));
    acceleration = add3(acceleration, scale3(steering, self->steering_weight));
    b->acceleration = clamplen3(acceleration, self->max_acceleration);
}

swarm_t swarm() {
    swarm_t self = {0};

    self.boids = NULL;

    self.perception_radius = 3; // 30

    self.separation_weight = 0.1; // 1
    self.separation_type = SWARM_DISTANCE_INVERSE_QUADRATIC;

    self.alignment_weight = 0.1; // 1
    self.cohesion_weight = 0.1; // 1

    self.steering_weight = 0.1; // 0.1
    // array_push(self.steering_targets, vec3(0,0,0));
    self.steering_target_type = SWARM_DISTANCE_LINEAR;

    self.blindspot_angledeg = 2; // 20
    self.max_acceleration = 1; // 10;
    self.max_velocity = 2; // 20;

    self.blindspot_angledeg_compare_value_ = 0; // = cos(M_PI * 2 * blindspot_angledeg / 360)
    map_init(self.voxel_cache_, less3, hash3);
    return self;
}

void swarm_update_acceleration_only(swarm_t *self) {
    self->perception_radius += !self->perception_radius; // 0->1

    // build voxel cache
    map_clear(self->voxel_cache_);
    for( int i = 0, end = array_count(self->boids); i < end; ++i ) {
        boid_t *b = &(self->boids)[i];

        vec3 *key = MALLOC(sizeof(vec3));
        *key = get_voxel_for_boid(self->perception_radius, b);
        array(boid_t*) *found = map_find_or_add_allocated_key( self->voxel_cache_, key, 0 );
        array_push(*found, b);
    }

    // update all boids
    for( int i = 0, end = array_count(self->boids); i < end; ++i ) {
        boid_t *b = &(self->boids)[i];
        update_boid(self, b);
    }
}

void swarm_update_acceleration_and_velocity_only(swarm_t *self, float delta) {
    self->blindspot_angledeg_compare_value_ = cosf(C_PI * 2 * self->blindspot_angledeg / 360.0f);
    swarm_update_acceleration_only(self);

    for( int i = 0, end = array_count(self->boids); i < end; ++i ) {
        boid_t *b = &(self->boids)[i];
        b->velocity = clamplen3(add3(b->velocity, scale3(b->acceleration, delta)), self->max_velocity);
    }
}

void swarm_update(swarm_t *self, float delta) {
    swarm_update_acceleration_and_velocity_only(self, delta);

    for( int i = 0, end = array_count(self->boids); i < end; ++i ) {
        boid_t *b = &(self->boids)[i];
        b->prev_position = b->position;
        b->position = add3(b->position, scale3(b->velocity, delta));
    }
}

int ui_swarm(swarm_t *self) {
    const char *distances[] = {
    "Linear",
    "Inverse Linear",
    "Quadratic",
    "Inverse Quadratic"
    };

    int rc = 0;
    rc |= ui_float( "Perception Radius", &self->perception_radius);
    ui_separator();

    rc |= ui_float( "Separation Weight", &self->separation_weight);
    rc |= ui_radio( "Separation Type", distances, countof(distances), (int*)&self->separation_type);
    ui_separator();

    rc |= ui_float( "Alignment Weight", &self->alignment_weight);
    rc |= ui_float( "Cohesion Weight", &self->cohesion_weight);
    ui_separator();

    rc |= ui_float( "Steering Weight", &self->steering_weight);
    //array(vec3) steering_targets;
    rc |= ui_radio( "Steering Target Type", distances, countof(distances), (int*)&self->steering_target_type);
    ui_separator();

    rc |= ui_float( "Blindspot Angle", &self->blindspot_angledeg);
    rc |= ui_float( "Max Acceleration", &self->max_acceleration);
    rc |= ui_float( "Max Velocity", &self->max_velocity);

    return rc;
}
