#define PROFILE 1
#ifndef PROFILE
#define PROFILE 0
#endif


u64 ReadSystemTimerMySec();
u64 ReadCPUTimer();


#if PROFILE == 1 // enable profiler

struct ProfilerBlock {
    u64 elapsed_atroot_tsc;
    u64 elapsed_tsc;
    u64 elapsed_children_tsc;
    const char *tag;
    u32 hits;
};
struct Profiler {
    u64 total_tsc;
    u64 total_systime;
    float cpu_freq;
    u32 count;
    u32 active;
    ProfilerBlock blocks[1024];
};
void ProfilerStart(Profiler *p) {
    p->total_systime = ReadSystemTimerMySec();
    p->total_tsc = ReadCPUTimer();
}
void ProfilerStop(Profiler *p) {
    p->total_systime = ReadSystemTimerMySec() - p->total_systime;
    p->total_tsc = ReadCPUTimer() - p->total_tsc;
    p->cpu_freq = (float) p->total_tsc / p->total_systime;
}
void ProfilerPrint(Profiler *p) {
    printf("\n");
    printf("Total time: %lu ms", p->total_systime / 1000);
    printf(" (tsc: %lu,", p->total_tsc);
    printf(" freq [tsc/mys]: %f)\n", p->cpu_freq);

    float pct_sum = 0;
    ProfilerBlock *current;
    for (int i = 1; i < p->count + 1; ++i) {
        current = p->blocks + i;
        printf("  %s: ", current->tag);

        u64 self_tsc = current->elapsed_tsc - current->elapsed_children_tsc;
        float self_pct = (float) self_tsc / p->total_tsc * 100;
        u64 total_tsc = current->elapsed_atroot_tsc;
        float total_pct = (float) total_tsc / p->total_tsc * 100;

        u32 hits = current->hits;
        printf("%lu (%.2f%%) self, %lu (%.2f%%) tot %u hits)\n", self_tsc, self_pct, total_tsc, total_pct, hits);
    }
}


class ProfileInitAndPrintMechanism {
public:
    Profiler *p;
    ProfileInitAndPrintMechanism(Profiler *p) {
        this->p = p;
        ProfilerStart(p);
    }
    ~ProfileInitAndPrintMechanism() {
        ProfilerStop(p);
        ProfilerPrint(p);
    }
};
class ProfileScopeMechanism {
public:
    Profiler *p;
    u32 slot;
    u64 start;
    u64 elapsed_atroot;
    u32 parent;
    ProfileScopeMechanism(Profiler *p, const char *tag, u32 slot) {
        this->p = p;

        this->start = ReadCPUTimer();
        this->elapsed_atroot = this->p->blocks[slot].elapsed_atroot_tsc;
        this->slot = slot;
        this->parent = this->p->active;

        this->p->blocks[slot].tag = tag;
        this->p->blocks[slot].hits += 1;

        this->p->count = MaxU32(this->p->count, slot);
        this->p->active = slot;
    }
    ~ProfileScopeMechanism() {
        u64 diff = ReadCPUTimer() - this->start;
        this->p->blocks[this->slot].elapsed_tsc += diff;
        this->p->blocks[this->slot].elapsed_atroot_tsc = this->elapsed_atroot + diff;
        
        if (this->parent) {
            this->p->blocks[this->parent].elapsed_children_tsc += diff;
        }
        this->p->active = parent;
    }
};


static Profiler g_prof;
#define TimeProgram ProfileInitAndPrintMechanism __prof_init__(&g_prof);
#define TimeFunction ProfileScopeMechanism __prof_mechanism__(&g_prof, __FUNCTION__, __COUNTER__ + 1);
#define TimeBlock(tag) ProfileScopeMechanism __prof_mechanism__(&g_prof, "<" tag ">", __COUNTER__ + 1);


#else // disable profiler, empty macros

// TODO: retain timing the entire program run as a very light-weight thing

#define TimeProgram ;
#define TimeFunction ;
#define TimeBlock(tag) ;

#endif
