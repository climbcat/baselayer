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

// ***********

struct Profiler {
    u64 total_tsc;
    u64 total_systime;
    float cpu_freq;
    u32 count;
};
struct ProfilerBlock {
    u64 elapsed_tsc;
    const char *tag;
    u32 hits;
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

    ProfilerBlock *current;
    for (int i = 1; i < g_prof.count + 1; ++i) {
        current = g_prof_blocks + i;
        printf("  %s: ", current->tag);
        printf("%lu (%.2f%% %u hits)\n", current->elapsed_tsc, (double) current->elapsed_tsc / p->total_tsc * 100, current->hits);
    }
}

class ProfileScopeMechanism {
    ProfilerBlock *block;
    u64 start;
public:
    ProfileScopeMechanism(const char *tag, u32 slot) {
        this->start = ReadCPUTimer();

        this->block = &g_prof_blocks[slot];
        this->block->tag = tag;
        this->block->hits += 1;
        g_prof.count = MaxU32(g_prof.count, slot);


    }
    ~ProfileScopeMechanism() {
        this->block->elapsed_tsc += ReadCPUTimer() - this->start;
    }
};

#define TimeProgram ProfilerStart(&g_prof);
#define TimeFunction ProfileScopeMechanism __prof_mechanism__(__FUNCTION__, __COUNTER__ + 1);
#define TimeBlock(tag) ProfileScopeMechanism __prof_mechanism__("<" tag ">", __COUNTER__ + 1);
#define TimePrint ProfilerStop(&g_prof); ProfilerPrint(&g_prof);
