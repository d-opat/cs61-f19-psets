

char* mystrstr(const char* s1, const char* s2) {
    //loop over s1
    while(*s1) {
        const char* s1try = s1;
        const char* s2try = s2;

        while( *s2try && *s2try == *s1try) {
            ++s2try;
            ++s1try;
        }

        //success if we get to the end of s2
        if (!*s2try) {
            return (char*)s1;
        }

        ++s1;
    }

    //special case
    if (!*s2) {
        return (char*) s1;
    }

    return nullptr;
}