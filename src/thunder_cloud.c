//FILE CREATED BY PERLITE ON 2025/08/12
#include "thunder_cloud.h"

#define ITEM_TERESA 11

//SOUND POTENTIAL : 73, 94, 233 (For the TC in await mode)

void init_thunder_inst(it_thunder_inst_t*);
void use_thunder_inst(void*, it_thunder_inst_t*);
void update_thunder_inst(it_thunder_inst_t*);
void await_thunder_inst(it_thunder_inst_t*);
void init_thunder_model(it_thunder_inst_t*);
void shocking_thunder_inst(it_thunder_inst_t*);
void shock_thunder_inst(it_thunder_inst_t*);
void killing_thunder_inst(it_thunder_inst_t*);
void emit_thunder_poof_particle(it_thunder_inst_t*);
void callback_PcEm_stay_at_inst(void*);
void render_thunder_inst(it_thunder_inst_t*, VecFx32*, void*);
void begin_await_thunder_inst(it_thunder_inst_t*);
void callback_PcEm_stay_at_drv(void*);

it_itemconfig_t cfg_thunder_cloud = {1,1,ITEM_TERESA, 1, 0,1, (void*)0x02102668};

it_item_def_t def_thunder_cloud = {
    sizeof(it_thunder_inst_t),
    1,
    1,
    -1,
    -1,
    NULL,
    init_thunder_inst,
    NULL,
    NULL,
    use_thunder_inst,
    update_thunder_inst,
    render_thunder_inst,
    (void*) 0x01FFD778,
    NULL,
    NULL,
    NULL,
    0,
    0,
    (void*)0x021004C4,
    0,
    init_thunder_model,
    1,0,0,0,
    0,
    0x4B0000,
    2,
    0,
    0,
    0,
    0x1000,
    0x1800,
    0x333,
    0x11f,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    1,
    0
}; //Most attributes copied from the base ghost definition structure

char it_teresa_nsbmd[16] ="it_teresa.nsbmd";
char it_teresa_nsbtp[16] ="it_teresa.nsbtp";
asm(R"(
    .thumb
    ncp_tcall(0x020F4D00)
        STR     R0, [R4]
        LDR     R5, =cfg_thunder_cloud

       @ LDR     R0, =0x1C0
        @ADDS    R4, R3, R0

        bx lr


    ncp_tcall(0x020F1BEC)
        LDR     R5, =def_thunder_cloud
        LDR     R0, =0x738

       @ ADDS    R4, R2, R0
        @MOVS    R3, #0x15

        bx lr
    
    
    ncp_tcall(0x020ED39C)
        ldr r0, =it_teresa_nsbmd
        ldr r1, =it_teresa_nsbtp
        bx lr
)");

void* state_table[] = {
        begin_await_thunder_inst,   await_thunder_inst, 
        shock_thunder_inst,         shocking_thunder_inst, 
        emit_thunder_poof_particle, killing_thunder_inst
        };

void init_thunder_inst(it_thunder_inst_t* inst){
    /*
    ENTRY   : TC inst
    ROLE    : Initializes the TC's struct and state machine
    VERSION : 2025AUG_04
    AUTHOR  : Perlite
    */
    
    sm_init(&inst->stateMachine, state_table, (int)(sizeof(state_table) / sizeof(void*)) >> 1, inst);
    inst->stateMachine.counter = 0;
    inst->lastHitCounter = 0;
    inst->chargeCounter = 0;
    inst->thunderBeat = 25;
    inst->frameTime = 0;
    inst->texID = 0;
};

void update_thunder_inst(it_thunder_inst_t* inst){
    /*
    ENTRY   : TC inst
    ROLE    : Sets the TC's X and Z position and prepares the Y position set by the current state of the TC. Also counts the age of the TC and executes the current TC state's function
    VERSION : 2025AUG_02
    AUTHOR  : Perlite
    */

    void* driver = driver_getById((int)inst->targetDriverId);
    VecFx32* drivPos = (VecFx32*)(driver + 0x80);
    VecFx32* drivDir = (VecFx32*)(driver + 0x50);
    VecFx32 target = {0};
    VEC_MultAdd(FX32_CONST(8), drivDir, drivPos, &target);

    inst->inst.position.x = inst->inst.position.x + FX_MUL(target.x - inst->inst.position.x, FX32_CONST(0.4));
    inst->preYPosition = inst->preYPosition + FX_MUL(target.y + FX32_CONST(32) - inst->preYPosition, FX32_CONST(0.4));
    inst->inst.position.z = inst->inst.position.z + FX_MUL(target.z - inst->inst.position.z, FX32_CONST(0.4));

    inst->frameTime += 1;

    sm_execute(&inst->stateMachine);
};

void begin_await_thunder_inst(it_thunder_inst_t* inst){
    /*
    ENTRY   : TC inst
    ROLE    : Sets the Y offset of particles during the Await state
    VERSION : 2025AUG_01
    AUTHOR  : Perlite
    */
    inst->particleOffset = FX32_CONST(1.7) * 4;
}

void await_thunder_inst(it_thunder_inst_t* inst){
    /*
    ENTRY   : TC inst
    ROLE    : Handles the base state of the TC by emitting particles, changing its texture and making it move slightly verically and changing slightly its scale depending on the state's counters and the TC's age. Also makes the TC change state after some time
    VERSION : 2025AUG_05
    AUTHOR  : Perlite
    */

    if (!inst->texPtcTime){
        inst->texID = 0;
        if (inst->particleEmitter[0]){
            ptcm_killEmitter(&inst->particleEmitter[0]);
            inst->particleEmitter[0] = NULL;
        }
        
    } else {
        inst->texID = 1;
        inst->texPtcTime -=1;
    }

    void* driver = driver_getById((int)inst->targetDriverId);

    inst->inst.position.y = inst->preYPosition + 2 * FX_SinIdx((u16)((inst->frameTime + 15) * 512));
    inst->inst.scale.x = FX32_CONST(1.7) + (FX_CosIdx((u16)(inst->frameTime * 1024)) >> 4);
    inst->inst.scale.y = FX32_CONST(1.7) + (FX_SinIdx((u16)(inst->frameTime * 1024)) >> 4);

    u16 hitMask = *(u16*)(driver + 0x2B2);
    for (int id = 0; id < driver_sDriverCount; id++){
        if (id != inst->targetDriverId && ((hitMask >> id) & 1) && inst->stateMachine.counter - inst->lastHitCounter > 30){
            inst->targetDriverId = id;
            inst->lastHitCounter = inst->stateMachine.counter;
        }
    }

    if (inst->stateMachine.counter - inst->chargeCounter >= 80)
        inst->chargeCounter = inst->stateMachine.counter;
    

    if ((inst->stateMachine.counter == inst->chargeCounter  && inst->stateMachine.counter >= 10) //10 is arbitrary, its just so its not too early
        || (inst->thunderBeat == 40 && inst->stateMachine.counter - inst->chargeCounter == 20)){

        if (inst->particleEmitter[0])
            ptcm_killEmitter(&inst->particleEmitter[0]);

        sfx_emit_item_inst(&inst->inst.sfx_emitter, 233, 127, 0);
        inst->texPtcTime = 10;

        inst->particleEmitter[0] = ptcm_createFogOffEmitter(1, &inst->inst.position);
        *(int*)(inst->particleEmitter[0] + 0x2C) -= FX32_CONST(1.7) >> 3;

        *(void**)(inst->particleEmitter[0] + 0x90) = callback_PcEm_stay_at_inst;
        *(it_thunder_inst_t**)(inst->particleEmitter[0] + 0x98) = inst;
        *(int*)(inst->particleEmitter[0] + 0x6C) = FX32_CONST(0.3);
    }
    
    if (inst->stateMachine.counter == 60*7) inst->thunderBeat = 40;

    if (inst->stateMachine.counter > 60*12 + 40) {
        sm_gotoState(&inst->stateMachine, 1);
    }
}

void use_thunder_inst(void* item_drag, it_thunder_inst_t* inst){
    /*
    ENTRY   : item_drag struct for the TC, TC inst
    ROLE    : Initializes the TC's sound emitter and position and state as the function is called when the TC is used
    VERSION : 2025AUG_02
    AUTHOR  : Perlite
    */
    it_init_item_inst_sound_emitter(inst);

    inst->targetDriverId = *(int*)(item_drag + 0x28);
    void* driver = driver_getById(inst->targetDriverId);
    VecFx32* drivPos = (VecFx32*)(driver + 0x80);
    inst->inst.position.x = drivPos->x;
    inst->preYPosition = drivPos->y;
    inst->inst.position.y = drivPos->y;
    inst->inst.position.z = drivPos->z;

    sm_gotoState(&inst->stateMachine, 0);
}

void init_thunder_model(it_thunder_inst_t* inst){
    /*
    ENTRY   : TC inst
    ROLE    : Initializes the TC's model with a function already present in the game
    VERSION : 2025AUG_02
    AUTHOR  : Perlite
    */

    it_item_model_coli_init(inst);
}

void shock_thunder_inst(it_thunder_inst_t* inst){
    /*
    ENTRY   : TC inst
    ROLE    : Shocks the driver and changes the TC's texture
    VERSION : 2025AUG_04
    AUTHOR  : Perlite
    */

    void* driver = driver_getById(inst->targetDriverId);
    race_startDarkening();
    
    driver_startThunderShrinkEffect(driver);

    if (inst->particleEmitter[0]){
        ptcm_killEmitter(&inst->particleEmitter[0]);
    }
    inst->particleEmitter[0] = ptcm_createFogOffEmitter(0, &inst->inst.position);
    *(void**)(inst->particleEmitter[0] + 0x90) = callback_PcEm_stay_at_inst;
    *(it_thunder_inst_t**)(inst->particleEmitter[0] + 0x98) = inst;

    inst->particleEmitter[1] = ptcm_createFogOffEmitter(1, &inst->inst.position);
    *(void**)(inst->particleEmitter[1] + 0x90) = callback_PcEm_stay_at_drv;
    *(it_thunder_inst_t**)(inst->particleEmitter[1] + 0x98) = inst;

    inst->particleOffset = -FX32_CONST(32 - 2 * 1.7);
    inst->texID = 1;
    sfx_emit_item_inst(&inst->inst.sfx_emitter, 233, 127, 0);
}

void shocking_thunder_inst(it_thunder_inst_t* inst){
    /*
    ENTRY   : TC inst
    ROLE    : Handles the TC in its shocking state by making it vibrate faster and changes the state after some time
    VERSION : 2025AUG_03
    AUTHOR  : Perlite
    */

    inst->inst.position.y = inst->preYPosition;
    inst->inst.scale.x = FX32_CONST(1.7) + (FX_CosIdx((u16)((inst->frameTime) * 4096)) >> 4);
    inst->inst.scale.y = FX32_CONST(1.7) + (FX_SinIdx((u16)((inst->frameTime) * 4096)) >> 4);

    if (inst->stateMachine.counter > 75){
        if (inst->particleEmitter[0]){
            ptcm_killEmitter(&inst->particleEmitter[0]);
            inst->particleEmitter[0] = NULL;
        }
        if (inst->particleEmitter[1]){
            ptcm_killEmitter(&inst->particleEmitter[1]);
            inst->particleEmitter[1] = NULL;
        }
        sm_gotoState(&inst->stateMachine, 2);
    }
}

void callback_PcEm_stay_at_inst(void* emitter){
    /*
    ENTRY   : Particle emitter
    ROLE    : Makes the PcEm stay at the TC instance
    VERSION : 2025AUG_02
    AUTHOR  : Perlite
    */

    it_thunder_inst_t* userWork = *(it_thunder_inst_t**)(emitter + 0x98);
    VecFx32* emit_pos =(VecFx32*)(emitter + 0x28);
    emit_pos->x = userWork->inst.position.x >> 4;
    emit_pos->y = (userWork->inst.position.y - userWork->particleOffset) >> 4;
    emit_pos->z = userWork->inst.position.z >> 4;
}

void callback_PcEm_stay_at_drv(void* emitter){
    /*
    ENTRY   : Particle emitter
    ROLE    : Makes the PcEm stay at the TC's driver
    VERSION : 2025AUG_01
    AUTHOR  : Perlite
    */

    it_thunder_inst_t* userWork = *(it_thunder_inst_t**)(emitter + 0x98);
    VecFx32* emit_pos =(VecFx32*)(emitter + 0x28);

    void* driver = driver_getById((int)userWork->targetDriverId);
    VecFx32* drivPos = (VecFx32*)(driver + 0x80);

    emit_pos->x = drivPos->x >> 4;
    emit_pos->y = drivPos->y >> 4;
    emit_pos->z = drivPos->z >> 4;
}

void emit_thunder_poof_particle(it_thunder_inst_t* inst){
    /*
    ENTRY   : TC inst
    ROLE    : Emits a cloud particle to allow a non offputting disappearance of the TC instance
    VERSION : 2025AUG_02
    AUTHOR  : Perlite
    */

    inst->particleEmitter[0] = ptcm_createEmitter(97, &inst->inst.position); //maybe 89 ? 97 ? 70 ?
    *(void**)(inst->particleEmitter[0] + 0x90) = callback_PcEm_stay_at_inst;
    *(it_thunder_inst_t**)(inst->particleEmitter[0] + 0x98) = inst;
    inst->particleOffset = 0;
}

void killing_thunder_inst(it_thunder_inst_t* inst){
    /*
    ENTRY   : TC inst
    ROLE    : Keeps the instance alive until the particle is thought to have finished its animation, then kills both the particle emitter and the instance
    VERSION : 2025AUG_01
    AUTHOR  : Perlite
    */

    void* driver = driver_getById((int)inst->targetDriverId);
    VecFx32* drivPos = (VecFx32*)(driver + 0x80);

    inst->inst.position.y = inst->preYPosition;
    
    if (inst->stateMachine.counter > 60) {
        if (inst->particleEmitter[0]){
            ptcm_killEmitterDirectSafe(&inst->particleEmitter[0]);
            inst->particleEmitter[0] = NULL;
        }
        it_itemsetDestroyItem((it_sItemsets + 0x44 * inst->inst.type), inst);
    }
}

void render_thunder_inst(it_thunder_inst_t* inst, VecFx32* pos, void* model){
    /*
    ENTRY   : TC inst, position vector, model
    ROLE    : Allows the TC instance to be rendered unless its being killed, handles texture changes and uses an already existing function to render the instance
    VERSION : 2025AUG_01
    AUTHOR  : Perlite
    */

    if (inst->stateMachine.curState != 2){
        *(s16*)(model + 0x18) = inst->texID;
        it_render_inst(inst, model, pos);
    }
}