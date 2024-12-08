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

bool CompileSoundDef(const char* source, const char* output) {
    bool success = true;
    std::ifstream src(source);
    if (!src.is_open()) return false;
    nlohmann::json json = nlohmann::json::parse(src);
    size_t output_size = 1; //Start with the number of entries (byte)
    struct SDEntry {
        std::string src;
        u8 num_buffers;
        u8 volume;
    };

    std::vector<SDEntry> entries;
    try {
        int i = 0;
        for (auto& snd : json["entries"]) {
            if (i > 255) {
                printf("More than 255 entries, trunking...\n");
                break;
            }
            const std::string& nm = snd["src"];
            output_size += 2 + 4 + nm.length() + 1; //num_buffers + volume + padding + string + 0x00
            entries.emplace_back(nm, snd["num_buffers"], snd["volume"]);
            i++;
        }
    }
    catch (const std::exception& e) {
        printf("An error ocurred: %s.\n", e.what());
        success = false;
    }
    if (success) {
        char* pBuffer = (char*)malloc(output_size);
        memset(pBuffer, 0x00, output_size);

        char* idx = pBuffer;
        idx[0] = entries.size();
        idx++;
        for (auto& s : entries) {
            size_t slen = s.src.length();
            memcpy(idx, s.src.data(), slen);
            idx += slen + 1;
            idx[0] = s.num_buffers;
            idx[1] = s.volume;
            idx += 2 + 4;
        }
        if (FILE* ot = fopen(output, "wb")) {
            fwrite(pBuffer, output_size, 1, ot);
            fclose(ot);
        }
        else {
            printf("Failed writing file.\n");
            success = false;
        }
        free(pBuffer);
    }
    return success;
}
