#ifndef TCP_INITIALIZER_H
#define TCP_INITIALIZER_H

namespace TCP {
    class Initializer {
    public:
        // may be initialized several times
        static void initialize();
        
        static void cleanup();
    };
}

#endif