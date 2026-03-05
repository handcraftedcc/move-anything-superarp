#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "host/midi_fx_api_v1.h"
#include "host/plugin_api_v1.h"

extern midi_fx_api_v1_t* move_midi_fx_init(const host_api_v1_t *host);

static void fail(const char *msg) {
    fprintf(stderr, "FAIL: %s\n", msg);
    exit(1);
}

static int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static uint32_t mix_u32(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

static uint32_t step_rand_u32(uint32_t seed, uint64_t step, uint32_t salt) {
    uint32_t lo = (uint32_t)(step & 0xFFFFFFFFu);
    uint32_t hi = (uint32_t)((step >> 32) & 0xFFFFFFFFu);
    return mix_u32(seed ^ lo ^ mix_u32(hi ^ salt) ^ salt);
}

static int rand_offset_signed(uint32_t r, int amount) {
    int span;
    if (amount <= 0) return 0;
    span = amount * 2 + 1;
    return (int)(r % (uint32_t)span) - amount;
}

static int expected_velocity(int base, int amount, int seed, uint64_t step) {
    uint32_t r = step_rand_u32((uint32_t)seed, step, 0xA11CEu);
    int delta = rand_offset_signed(r, amount);
    return clamp_int(base + delta, 1, 127);
}

static int find_seed_with_distinct_steps(int amount, uint64_t step_a, uint64_t step_b) {
    int seed;
    for (seed = 0; seed <= 65535; seed++) {
        int va = expected_velocity(100, amount, seed, step_a);
        int vb = expected_velocity(100, amount, seed, step_b);
        if (va != vb) return seed;
    }
    return -1;
}

static void send_note(midi_fx_api_v1_t *api, void *inst, uint8_t status, uint8_t note, uint8_t vel) {
    uint8_t in_msg[3];
    uint8_t out_msgs[8][3];
    int out_lens[8];
    in_msg[0] = status;
    in_msg[1] = note;
    in_msg[2] = vel;
    (void)api->process_midi(inst, in_msg, 3, out_msgs, out_lens, 8);
}

static int wait_for_note_on_velocity(midi_fx_api_v1_t *api, void *inst) {
    int tries, i;
    uint8_t out_msgs[64][3];
    int out_lens[64];
    for (tries = 0; tries < 16; tries++) {
        int count = api->tick(inst, 1500, 48000, out_msgs, out_lens, 64);
        for (i = 0; i < count; i++) {
            uint8_t st = out_msgs[i][0] & 0xF0;
            if (st == 0x90 && out_msgs[i][2] > 0) {
                return (int)out_msgs[i][2];
            }
        }
    }
    fail("timed out waiting for NOTE_ON");
    return -1;
}

static void configure_for_test(midi_fx_api_v1_t *api, void *inst, const char *mode, int seed) {
    char seed_buf[16];
    api->set_param(inst, "sync", "internal");
    api->set_param(inst, "rate", "1/32");
    api->set_param(inst, "bpm", "240");
    api->set_param(inst, "velocity", "100");
    api->set_param(inst, "velocity_random_amount", "50");
    api->set_param(inst, "modifier_loop_length", "3");
    api->set_param(inst, "modifier_trigger", mode);
    snprintf(seed_buf, sizeof(seed_buf), "%d", seed);
    api->set_param(inst, "velocity_seed", seed_buf);
}

static void assert_mode_roundtrip(midi_fx_api_v1_t *api, void *inst) {
    char buf[64];
    int len;

    memset(buf, 0, sizeof(buf));
    len = api->get_param(inst, "modifier_trigger", buf, sizeof(buf));
    if (len <= 0 || strcmp(buf, "retrigger") != 0) {
        fail("default modifier_trigger should be retrigger");
    }

    api->set_param(inst, "modifier_trigger", "continuous");
    memset(buf, 0, sizeof(buf));
    len = api->get_param(inst, "modifier_trigger", buf, sizeof(buf));
    if (len <= 0 || strcmp(buf, "continuous") != 0) {
        fail("modifier_trigger should accept continuous");
    }
}

static void run_phrase_transition_case(midi_fx_api_v1_t *api, void *inst, int expect_after, int expect_first) {
    int first_vel, second_vel, next_phrase_vel;

    send_note(api, inst, 0x90, 60, 100);
    first_vel = wait_for_note_on_velocity(api, inst);
    second_vel = wait_for_note_on_velocity(api, inst);
    (void)second_vel;
    send_note(api, inst, 0x80, 60, 0);

    send_note(api, inst, 0x90, 62, 100);
    next_phrase_vel = wait_for_note_on_velocity(api, inst);
    send_note(api, inst, 0x80, 62, 0);

    if (first_vel != expect_first) {
        fail("first phrase velocity did not match expected step 0");
    }
    if (next_phrase_vel != expect_after) {
        fail("next phrase velocity did not match expected modifier trigger behavior");
    }
}

int main(void) {
    host_api_v1_t host;
    midi_fx_api_v1_t *api;
    void *inst;
    int seed;
    int step0_vel;
    int step2_vel;

    memset(&host, 0, sizeof(host));
    host.api_version = MOVE_PLUGIN_API_VERSION;

    api = move_midi_fx_init(&host);
    if (!api || !api->create_instance || !api->set_param || !api->get_param || !api->destroy_instance || !api->tick || !api->process_midi) {
        fail("superarp API init/callbacks missing");
    }

    seed = find_seed_with_distinct_steps(50, 0, 2);
    if (seed < 0) fail("unable to find suitable seed");
    step0_vel = expected_velocity(100, 50, seed, 0);
    step2_vel = expected_velocity(100, 50, seed, 2);
    if (step0_vel == step2_vel) fail("test setup expects distinct velocities for step 0 and 2");

    inst = api->create_instance(".", NULL);
    if (!inst) fail("create_instance returned NULL");
    assert_mode_roundtrip(api, inst);
    api->destroy_instance(inst);

    inst = api->create_instance(".", NULL);
    if (!inst) fail("create_instance returned NULL for retrigger test");
    configure_for_test(api, inst, "retrigger", seed);
    run_phrase_transition_case(api, inst, step0_vel, step0_vel);
    api->destroy_instance(inst);

    inst = api->create_instance(".", NULL);
    if (!inst) fail("create_instance returned NULL for continuous test");
    configure_for_test(api, inst, "continuous", seed);
    run_phrase_transition_case(api, inst, step2_vel, step0_vel);
    api->destroy_instance(inst);

    printf("PASS: superarp modifier trigger mode\n");
    return 0;
}
