//FILE CREATED BY PERLITE ON 2025/08/12
#include "mkds.h"


typedef struct
{
    u32 instanceSize; // Use sizeof 
    u32 limit; //max amount of item instances allows
    u32 field8; //unsure on the purpose of this Possible values are:

    int instanceCount; //-1 on every item probably set somewhere else
    int field10; //-1 on every item probably set somewhere else
    void* loadFunc; //LoadFunction used in kouraW to load the model also used in most of the single use items
    void* initInstanceFunc; //Similar to the init functions on mapobjs
    void* field1C;
    void* field20;
    void* deployFunc; //used by gesso to play the deploy sound, called when the player stops dragging the ittem
    void* updateFunc;
    void* renderFunc;
    void* visibilityFlagCalcFunc; //Used to calculate if an object should be visible
    void* field34;
    void* field38;
    void* destroyInstFunc;
    int field40;
    int field44;
    void* field48;
    int field4C;
    void* field50;
    u8 gap54;
    u8 gap55;
    u8 gap56;
    u8 gap57;
    void* field58; //related to sound emission?
    int field5C;
    int field60;
    int colSphereRadius;
    int sphereRadius1;
    int sphereRadius2;
    int field70;
    int scale;
    int field78;
    int field7C;
    int field80;
    int field84;
    int field88;
    u32 gap8C;
    int field90;
    int field94;
    int field98;
    int field9C;
    int fieldA0;
    int fieldA4;
} it_item_def_t; //Base Game item definition structure. Copied from Rocoloco321's research document 


typedef struct
{
    int enabled; //Will not appear on regular gameplay if false???
    int wifiEnabled;  //Will not appear on Wifi if false
    int type; //Object type, spawns the object id that is given
    u32 count; // Amount of times it can be used???
    u32 field10; //Unsure on what this do. 0 on every item minus the golden mushroom
    u32 field14;// possible values are 0-4
    void* activateFunc; //Activation function, called when the item is used, seems to be used only on items that have special behavior, 0 when it’s unused
} it_itemconfig_t; //Base Game item config structure. Copied from Rocoloco321's research document 

typedef struct
{
    char sfx_emitter[0x44];
    u32 type;
    u32 field48;
    u16 field4C;
    u16 field4E;
    VecFx32 position;
    VecFx32 velocity;
    VecFx32 scale;
    u32 flags;
    u16 field78;
    u16 field7A;
    char light[0x14];
    void* lightPTR;
    char mtx[0x3C];
    u32 fieldD0;
    u32 visibilityFlags;
    u16 alpha;
    u16 colEntryId;
    u32 fieldDC;
    u32 sphereSize;
    VecFx32 fieldE4;
    VecFx32 fieldF0;
    u32 fieldFC;
    u32 field100;
    u32 field104;
    VecFx32 field108;
    u16 field114;
    u16 field116;
    u16 field118;
    u16 field11A;
    u32 field11C;
    void* update_func_IDK;
    u32 field124;
    u16 field128;
    u16 field12B;
} it_item_inst_t; //Base Game parent item instance structure

typedef struct{
    void** pStates;
    u32 counter;
    void* pUserData;
    u16 nrStates;
    u16 curState;
    u16 nextState;
    u16 gotoNextState;
} state_machine_t; //Base Game state machine structure

typedef struct{
    it_item_inst_t inst;
    u16 targetDriverId;
    u16 thunderBeat;
    u16 lastHitCounter;
    u16 chargeCounter;
    u16 age;
    u16 particleOffset;
    u16 texPtcTime;
    u16 texID;
    state_machine_t stateMachine;
    void* particleEmitter[2];
    int preYPosition;
} it_thunder_inst_t; //Children structure of the item instance structure

//External Variables
extern void* it_sItemsets;
extern s16 driver_sDriverCount;
extern void* mobj_calcVisibilityFlags2d;
extern void* r2d_sStatus; 

//External Functions
extern void sub_20F8E78(it_item_inst_t*);
extern void sm_execute(state_machine_t*);
extern void sm_gotoState(state_machine_t*, int);
extern void it_itemsetDestroyItem(void*, it_item_inst_t*);
extern void sm_init(state_machine_t*, void*, int, it_thunder_inst_t*);
extern void sub_2082E4C(u16);
extern void race_startDarkening();
extern void driver_startThunderShrinkEffect(void*);
extern void ptcm_killEmitterDirectSafe(void*);
extern void ptcm_killEmitter(void*);
extern void* ptcm_createFogOffEmitter(int, VecFx32*);
extern void* ptcm_createEmitter(int, VecFx32*);
extern void sfx_21090C4(void*,int,char,short);
extern void sub_20F9114(it_item_inst_t*);
extern void VEC_MultAdd(int, VecFx32*, VecFx32*, VecFx32*);
extern void sub_20ED5C0(it_item_inst_t*, void*, VecFx32*);

//Renaming of certain External Functions
#define it_item_model_coli_init sub_20F8E78
#define sfx_emit_item_inst sfx_21090C4
#define it_init_item_inst_sound_emitter sub_20F9114
#define it_render_inst sub_20ED5C0


