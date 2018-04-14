#include "bbzzooids.h"

bbzvm_t vmObj;
Message bbzmsg_tx;
uint8_t bbzmsg_buf[11];
bbzmsg_payload_t bbz_payload_buf;

extern Position robotPosition;
extern float robotOrientation;
bbzheap_idx_t pos_x_idx;
bbzheap_idx_t pos_y_idx;
bbzheap_idx_t pos_orientation_idx;

extern Motor motorValues;
extern Target currentGoal;

uint8_t buf[4];
const uint8_t *bbzzooids_bcodeFetcher(bbzpc_t offset, uint8_t size)
{
    intptr_t __addr16 = (intptr_t)((intptr_t)((intptr_t)&bcode + sizeof(*bcode) * offset));
    for (uint8_t i = 0; i < size; ++i) {
        *(uint8_t *)(buf + i) = *(uint8_t *)(__addr16 + i);
    }
    return buf;
}

void bbz_func_call(uint16_t strid) {
    bbzvm_pushs(strid);
    bbzheap_idx_t l = bbzvm_stack_at(0);
    bbzvm_pop();
    if(bbztable_get(vmObj.gsyms, l, &l)) {
        bbzvm_pushnil(); // Push self table
        bbzvm_push(l);
        bbzvm_closure_call(0);
    }
}

Message* bbzwhich_msg_tx() {
#ifndef BBZ_DISABLE_MESSAGES
    if(bbzoutmsg_queue_size()) {
        bbzoutmsg_queue_first(&bbz_payload_buf);
        bbzmsg_tx.header.type = TYPE_BBZ_MESSAGE;
        bbzmsg_tx.header.id = 255;
        *(uint8_t*)bbzmsg_tx.payload = getRobotId();
        *(Position*)(bbzmsg_tx.payload + sizeof(uint8_t)) = *getRobotPosition();
        for (uint8_t i=0;i<9;++i) {
            bbzmsg_tx.payload[i+sizeof(Position)+sizeof(uint8_t)] = *bbzringbuf_at(&bbz_payload_buf, i);
        }
        return &bbzmsg_tx;
    }
#endif
    return 0;
}

void bbzprocess_msg_rx(Message* msg_rx, uint16_t distance) {
#ifndef BBZ_DISABLE_MESSAGES
    if (msg_rx->header.type == TYPE_BBZ_MESSAGE) {
        bbzringbuf_clear(&bbz_payload_buf);
        for (uint8_t i = 0; i < 9; ++i) {
            *bbzringbuf_rawat(&bbz_payload_buf, bbzringbuf_makeslot(&bbz_payload_buf)) = msg_rx->payload[i + sizeof(Position) + sizeof(uint8_t)];
        }
        // Add the neighbor data.
#ifndef BBZ_DISABLE_NEIGHBORS
        if (*bbzmsg_buf == BBZMSG_BROADCAST) {
            bbzneighbors_elem_t elem;
            elem.azimuth = 0;
            elem.elevation = 0;
            elem.robot = *(uint8_t*)msg_rx->payload;
            elem.distance = distance >> 8;
            bbzneighbors_add(&elem);
        }
#endif // !BBZ_DISABLE_NEIGHBORS
        bbzinmsg_queue_append(&bbz_payload_buf);
    }
#endif // !BBZ_DISABLE_MESSAGES
}

void bbz_init(void)
{
    initRobot();
    //setGreenLed(5);
    //setMotor1(30);
    //setMotor2(30);
    vm = &vmObj;
    bbzringbuf_construct(&bbz_payload_buf, bbzmsg_buf, 1, 11);
}

void bbz_createPosObject() {
    bbzvm_pusht(); // "pos"
    pos_x_idx = bbzint_new(0);
    pos_y_idx = bbzint_new(0);
    pos_orientation_idx = bbzfloat_new(bbzfloat_fromint(0));
    bbztable_add_data(__BBZSTRID_x, pos_x_idx);
    bbztable_add_data(__BBZSTRID_y, pos_y_idx);
    bbztable_add_data(__BBZSTRID_orientation, pos_orientation_idx);
    bbzvm_gsym_register(__BBZSTRID_pos, bbzvm_stack_at(0));
    bbzheap_obj_make_permanent(*bbzheap_obj_at(bbzvm_stack_at(0)));
    bbzvm_pop();
}

void bbz_updatePosObject() {
    bbzvm_assign(&bbzheap_obj_at(pos_x_idx)->i.value, &robotPosition.x);
    bbzvm_assign(&bbzheap_obj_at(pos_y_idx)->i.value, &robotPosition.y);
    bbzheap_obj_at(pos_orientation_idx)->f.value = bbzfloat_fromfloat(robotOrientation);
}

void bbz_start(void (*setup)(void))
{
    uint8_t has_setup = 0, init_done = 0;
    while (1)
    {
        checkRadio();
        // checkTouch();
        if (!init_done) {
            if (!has_setup) {
                bbzvm_construct(getRobotId());
                bbzvm_set_bcode(bbzzooids_bcodeFetcher, bcode_size);
                bbzvm_set_error_receiver(bbz_err_receiver);
                bbz_createPosObject();
                setup();
                has_setup = 1;
            }
            if (vm->state == BBZVM_STATE_READY) {
                bbzvm_step();
            }
            else {
                init_done = 1;
                vm->state = BBZVM_STATE_READY;
                bbz_func_call(__BBZSTRID_init);
#ifndef BBZ_DISABLE_MESSAGES
                message_tx = bbzwhich_msg_tx;
                message_tx_success = bbzoutmsg_queue_next;
                message_rx = bbzprocess_msg_rx;
#endif
            }
        }
        else {
            if (vm->state != BBZVM_STATE_ERROR) {
                bbzvm_process_inmsgs();
                bbz_func_call(__BBZSTRID_step);
                bbzvm_process_outmsgs();
            }
            //updateRobot();
            updateRobotPosition();
            bbz_updatePosObject();
        }
    }
}

void set_color(uint8_t rgb) {
    setRGBLed((rgb&3) << 4, (rgb&0xc) << 2, (rgb&0x30));
}
static void ___led(uint8_t x)
{
    set_color(x);
    delay(50);
    set_color(0);
    delay(100);
}
void bbz_err_receiver(bbzvm_error errcode)
{
    setMotor1(0);
    setMotor2(0);
    uint8_t i;
    ___led(RGB(1, 2, 0));
    ___led(RGB(1, 2, 0));
    ___led(RGB(1, 2, 0));
    ___led(RGB(1, 2, 0));
    delay(300);
#if 0
    for (i = 4; i; --i)
    {
        delay(700);
        switch (errcode)
        {
        case BBZVM_ERROR_INSTR:
            ___led(RGB(2, 0, 0));
            ___led(RGB(2, 0, 0));
            break;
        case BBZVM_ERROR_STACK:
            ___led(RGB(1, 2, 0));
            if (bbzvm_stack_size() >= BBZSTACK_SIZE)
            {
                ___led(RGB(0, 3, 0));
            }
            else if (bbzvm_stack_size() <= 0)
            {
                ___led(RGB(2, 0, 0));
            }
            else
            {
                ___led(RGB(1, 2, 0));
            }
            break;
        case BBZVM_ERROR_LNUM:
            ___led(RGB(3, 1, 0));
            ___led(RGB(3, 1, 0));
            break;
        case BBZVM_ERROR_PC:
            ___led(RGB(0, 3, 0));
            ___led(RGB(0, 3, 0));
            break;
        case BBZVM_ERROR_FLIST:
            ___led(RGB(0, 3, 0));
            ___led(RGB(2, 0, 0));
            break;
        case BBZVM_ERROR_TYPE:
            ___led(RGB(0, 3, 0));
            ___led(RGB(1, 2, 0));
            break;
        case BBZVM_ERROR_OUTOFRANGE:
            ___led(RGB(0, 0, 2));
            ___led(RGB(2, 0, 0));
            break;
        case BBZVM_ERROR_NOTIMPL:
            ___led(RGB(0, 0, 2));
            ___led(RGB(0, 2, 0));
            break;
        case BBZVM_ERROR_RET:
            ___led(RGB(0, 3, 0));
            ___led(RGB(0, 0, 2));
            break;
        case BBZVM_ERROR_STRING:
            ___led(RGB(0, 2, 1));
            ___led(RGB(0, 2, 1));
            break;
        case BBZVM_ERROR_SWARM:
            ___led(RGB(0, 2, 1));
            ___led(RGB(2, 0, 0));
            break;
        case BBZVM_ERROR_VSTIG:
            ___led(RGB(0, 2, 1));
            ___led(RGB(1, 2, 0));
            break;
        case BBZVM_ERROR_MEM:
            ___led(RGB(0, 2, 1));
            ___led(RGB(0, 0, 2));
            break;
        case BBZVM_ERROR_MATH:
            ___led(RGB(0, 0, 2));
            ___led(RGB(0, 0, 2));
            break;
        default:
            ___led(RGB(2, 0, 2));
            ___led(RGB(2, 0, 2));
            break;
        }
    }
#endif
    ___led(RGB(2, 2, 2));
    ___led(RGB(2, 2, 2));
    Reboot();
}

static uint8_t seed = 0xaa, accumulator = 0;

uint8_t rand_soft()
{
    seed ^= seed << 3;
    seed ^= seed >> 5;
    seed ^= accumulator++ >> 2;
    return seed;
}

void rand_seed(uint8_t s)
{
    seed = s;
}

void set_motors(int8_t m1, int8_t m2)
{
#ifndef DEBUG
    setMotor1(m1);
    setMotor2(m2);
#endif
}

void spinup_motors()
{
#ifndef DEBUG
    set_motors(MAX_SPEED, MAX_SPEED);
    delay(15);
#endif
}