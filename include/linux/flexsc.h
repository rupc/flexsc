struct flexsc_syspage;

struct flexsc_sysentry {
    long args[6];
    unsigned nargs;
    unsigned short rstatus;
    unsigned short sysnum;
    unsigned sysret;
} ____cacheline_aligned_in_smp;
