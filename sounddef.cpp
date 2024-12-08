#include "sounddef.hpp"

bool DumpSoundDef(const char* file) {
    bool success = true;
    size_t size;
    int idx = 1;
    if (u8* pSDData = (u8*)GetDataFromFile(file, size)) {
        u8 num_sounds = pSDData[0] - 1;
        printf("{\n");
        printf("    \"entries\" : [\n");
        for (int i = 0; i <= num_sounds; i++) {
            printf("        {\n");
            printf("            \"src\" : \"%s\",\n", pSDData + idx);
            idx += strnlen((const char*)pSDData + idx, size) + 1;
            printf("            \"num_buffers\" : %d,\n", pSDData[idx]);
            printf("            \"volume\" : %d\n", pSDData[idx + 1]);
            idx += 6;
            printf("        }%c\n", (i < num_sounds) ? ',' : ' ');
        }
        printf("    ]\n");
        printf("}\n");
    }
    return success;
}
