bool isalpha(unsigned char c) {
    return (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'));
}

unsigned char tolower(unsigned char c) {
    if('A' <= c && c <= 'Z')
    {
        return int('a') + c - 'A';
    }
    return c;
}
