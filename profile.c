u64 ReadSystemTimerMySec() {
    u64 systime;
    struct timeval tm;
    gettimeofday(&tm, NULL);
    systime = (u32) tm.tv_sec*1000000 + tm.tv_usec; // microsecs 

    return systime;
}
u64 ReadCPUTimer() {
    u64 ticks = __builtin_ia32_rdtsc(); // gcc
    return ticks;
}

struct Profiler {
    u64 total_tsc;
    u64 total_systime;
    float cpu_freq;
    u32 count;
    u32 active;
};
struct ProfilerBlock {
    u64 elapsed_tsc;
    u64 elapsed_children_tsc;
    const char *tag;
    u32 hits;
    u32 parent;
};
static Profiler g_prof;
static ProfilerBlock g_prof_blocks[1024];

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
    printf("Total time: %lu mysec", p->total_systime);
    printf(" (tsc: %lu,", p->total_tsc);
    printf(" freq [tsc/mys]: %f)\n", p->cpu_freq);

    float pct_sum = 0;
    ProfilerBlock *current;
    for (int i = 1; i < g_prof.count + 1; ++i) {
        current = g_prof_blocks + i;
        printf("  %s: ", current->tag);

        u64 elapsed_tsc = current->elapsed_tsc - current->elapsed_children_tsc;
        float elapsed_pct = (float) elapsed_tsc / p->total_tsc * 100;
        u32 hits = current->hits;
        printf("%lu (%.2f%% %u hits)\n", elapsed_tsc, elapsed_pct, hits);

        pct_sum += elapsed_pct;
    }
    printf("%%sum: %f\n", pct_sum);
}

class ProfileScopeMechanism {
public:
    u32 slot;
    u64 start;
    ProfileScopeMechanism(const char *tag, u32 slot) {
        this->start = ReadCPUTimer();
        this->slot = slot;

        g_prof_blocks[slot].tag = tag;
        g_prof_blocks[slot].hits += 1;
        g_prof_blocks[slot].parent = g_prof.active;

        g_prof.count = MaxU32(g_prof.count, slot);
        g_prof.active = slot;
    }
    ~ProfileScopeMechanism() {
        u64 diff = ReadCPUTimer() - this->start;
        g_prof_blocks[this->slot].elapsed_tsc += diff;

        u32 parent = g_prof_blocks[this->slot].parent;
        if (parent) {
            g_prof_blocks[parent].elapsed_children_tsc += diff;
        }
        g_prof.active = parent;        
    }
};

#define TimeProgram ProfilerStart(&g_prof);
#define TimeFunction ProfileScopeMechanism __prof_mechanism__(__FUNCTION__, __COUNTER__ + 1);
#define TimeBlock(tag) ProfileScopeMechanism __prof_mechanism__("<" tag ">", __COUNTER__ + 1);
#define TimePrint ProfilerStop(&g_prof); ProfilerPrint(&g_prof);
