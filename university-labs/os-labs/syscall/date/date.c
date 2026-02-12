#include "types.h"
#include "user.h"
#include "date.h"

int main(int argc, char *argv[]) {
    struct rtcdate cur;
    int ok = date(&cur);
    if(ok == -1)
    {
        exit();
    }
    printf(1, "%d-%d-%dT%d:%d:%d\n", cur.year, cur.month, cur.day, cur.hour, cur.minute, cur.second);
    exit();
}
