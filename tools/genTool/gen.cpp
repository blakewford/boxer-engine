#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "boxer.h"

using namespace boxer;

void strip(char* buffer, char* dest)
{
    char* k = buffer;
    while(isspace(*k))
        k++;
    memcpy(dest, k, strlen(k));
    memset(buffer, '\0', MAX_MANIFEST_TEXT);
}

int32_t globalSize = 0;
char globalPath[MAX_MANIFEST_ENTRY*MAX_MANIFEST_TEXT];

void removeQuote(char* temp, const char* value)
{
    memset(temp, '\0', MAX_MANIFEST_TEXT);

    int32_t k = 0;
    int32_t l = 0;
    const int32_t count = strlen(value);
    while(k < count)
    {
        if(value[k] != '\"')
        {
            temp[l++] = value[k];
        }
        k++;
    }
}

void output(const char* value)
{
    char temp[MAX_MANIFEST_TEXT];
    removeQuote(temp, value);
    int32_t count = strlen(temp);
    while(count--)
    {
        temp[count] = toupper(temp[count]);
    }
    printf("#define %s %d\n", temp, globalSize);
    globalSize++;
}

void push(char** storage, const char* key)
{
    int32_t keyLength = strlen(key);
    memcpy(*storage, key, keyLength);
    *storage+=keyLength;
    *storage+=2;
}

void parser(uint8_t* text)
{
    char* storage = globalPath;

    int32_t i = 0;
    int32_t j = 0;
    bool first = true;
    char current = text[i];
    char key[MAX_MANIFEST_TEXT];
    char value[MAX_MANIFEST_TEXT];
    char buffer[MAX_MANIFEST_TEXT];
    memset(buffer, '\0', MAX_MANIFEST_TEXT);
    while(current != '\0')
    {
        if(current == '{')
        {
        }
        else if(current == '}')
        {
            break;
        }
        else if(current == ':')
        {
            memset(key, '\0', MAX_MANIFEST_TEXT);
            strip(buffer, key);
            if(first)
            {
                assert(!strcmp(key, "\"manifest_version\""));
            }
            else
            {
                push(&storage, key);
            }
            j = 0;
        }
        else if(current == ',')
        {
            memset(value, '\0', MAX_MANIFEST_TEXT);
            if(first)
            {
                first = false;
                assert(atoi(buffer) == 1);
            }
            else
            {
                memset(value, '\0', MAX_MANIFEST_TEXT);
                strip(buffer, value);
                output(value);
            }
            j = 0;
        }
        else
        {
            buffer[j++] = current;
        }
        current = text[++i];
    }
    j--;
    buffer[j] = '\0';
    memset(value, '\0', MAX_MANIFEST_TEXT);
    strip(buffer, value);
    push(&storage, key);
    output(value);

    storage = globalPath;
    printf("int32_t _BOXER_FILES_SIZE = %d;\n", globalSize);
    printf("const char* _BOXER_FILES[%d] = {\n", globalSize);
    while(globalSize--)
    {
        int32_t length = strlen(storage);
        char temp[MAX_MANIFEST_TEXT];
        removeQuote(temp, storage);
        printf("\"%s\",\n", temp);
        storage+=length;
        storage+=2;
    }
    printf("};\n");
}

int main(int argc, char** argv)
{
    memset(globalPath, '\0', MAX_MANIFEST_ENTRY*MAX_MANIFEST_TEXT);

    FILE* manifest = fopen("manifest", "r");
    if(manifest)
    {
        fseek(manifest, 0, SEEK_END);
        int32_t size = ftell(manifest);
        rewind(manifest);
        uint8_t* text = (uint8_t*)malloc(size);
        size_t read = fread(text, 1, size, manifest);
        assert(read == size);
        fclose(manifest);

        parser(text);
        free(text);
    }
    else
    {
        printf("Could not find manifest\n");
    }

    return 0;
}
