// TODO: make independent on arena
// TODO: make independent on string
// TODO: include gettimeofday currently in utils.c

u64 ReadSystemTimerMySec() {
    u64 systime;
    struct timeval tm;
    gettimeofday(&tm, NULL);
    systime = (u32) tm.tv_sec*1000000 + tm.tv_usec; // microsecs 

    return systime;
}

u64 ReadCPUTimer() {
    // gcc:
    u64 ticks = __builtin_ia32_rdtsc();
    return ticks;
}

struct ProfilerBlock {
    ProfilerBlock *next = NULL;
    String name;
    u64 tsc_diff = 0;
};

struct Profiler {
    MArena arena;
    ProfilerBlock *first = NULL;
    ProfilerBlock *current = NULL;

    u64 cputime_diff = 0;
    u64 systime_diff = 0;
    float cpu_freq = 0;
};

Profiler ProfilerInit() {
    ProfilerBlock block;
    Profiler prof;
    prof.arena = ArenaCreate();
    prof.first = (ProfilerBlock*) ArenaPush(&prof.arena, &block, sizeof(block));
    prof.current = prof.first;
    prof.systime_diff = ReadSystemTimerMySec();
    prof.cputime_diff = ReadCPUTimer();

    return prof;
}

ProfilerBlock *ProfilerReadBlockStart(Profiler *p, const char *func_name) {
    ProfilerBlock *block = (ProfilerBlock*) ArenaPush(&p->arena, &block, sizeof(ProfilerBlock));
    p->current = (ProfilerBlock*) InsertAfter1(p->current, block);
    p->current->next = NULL;
    p->current->name = StrLiteral(&p->arena, func_name);
    p->current->tsc_diff = ReadCPUTimer();
    return p->current;
}
void ProfilerReadBlockEnd(Profiler *p, ProfilerBlock *block) {
    block->tsc_diff = ReadCPUTimer() - block->tsc_diff;
}

void ProfilerStop(Profiler *p) {
    p->systime_diff = ReadSystemTimerMySec() - p->systime_diff;
    p->cputime_diff = ReadCPUTimer() - p->cputime_diff;
    p->cpu_freq = (float) p->cputime_diff / p->systime_diff;
    p->current = NULL;
}
void ProfilerPrint(Profiler *p) {
    ProfilerBlock *current = p->first->next;
    printf("\n");
    printf("Total time: %lu mysec", p->systime_diff);
    printf(" (tsc: %lu,", p->cputime_diff);
    printf(" freq [tsc/mys]: %f)\n", p->cpu_freq);
    while (current != NULL) {
        StrPrint("  %s: ", current->name);
        printf("%lu (%f %%)\n", current->tsc_diff, (double) current->tsc_diff / p->cputime_diff * 100);
        current = current->next;
    }
}

Profiler g_prof = ProfilerInit();
class ProfileScopeMechanism {
    ProfilerBlock *block;
public:
    ProfileScopeMechanism(const char * func_name) {
        this->block = ProfilerReadBlockStart(&g_prof, func_name);
    }
    ~ProfileScopeMechanism() {
        ProfilerReadBlockEnd(&g_prof, this->block);
    }
};

#define TimeFunction ProfileScopeMechanism __prof_mechanism__(__FUNCTION__);
#define TimeBlock(name) ProfileScopeMechanism __prof_mechanism__(name);
#define TimePrint ProfilerStop(&g_prof); ProfilerPrint(&g_prof);
