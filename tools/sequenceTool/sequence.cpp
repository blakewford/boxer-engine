#include <list>
#include <string>
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <algorithm>

std::list<std::string> gList;
#define PARSE(x) value.compare(0, strlen(x), x) == 0

void rotate()
{
    std::string original;
    std::string angle;
    std::string stepString;
    std::string startIndex;
    printf("Original Image?\n");
    std::cin >> original;
    printf("Rotation Angle?\n");
    std::cin >> angle;
    printf("Number of Steps?\n");
    std::cin >> stepString;
    printf("Start Index?\n");
    std::cin >> startIndex;

    char buffer[1024];
    memset(buffer, '\0', 1024);
    sprintf(buffer, "cp %s /tmp/%s.bmp", original.c_str(), startIndex.c_str());
    int32_t status = system(buffer);
    assert(status == 0);

    char alias[64];
    memset(alias, '\0', 64);
    sprintf(alias, "/tmp/%d.bmp", atoi(startIndex.c_str()));
    gList.push_back(alias);

    const int32_t steps = atoi(stepString.c_str());
    const int32_t start = atoi(startIndex.c_str());
    for(int32_t i=start; i < start+steps; i++)
    {
        memset(buffer, '\0', 1024);
        sprintf(buffer, "convert /tmp/%d.bmp +antialias -rotate %d /tmp/%d.bmp", i, (atoi(angle.c_str())/steps), i+1);

        memset(alias, '\0', 64);
        sprintf(alias, "/tmp/%d.bmp", i+1);
        std::string temp(alias);
        gList.push_back(temp);
        status = system(buffer);
        assert(status == 0);
    }

    printf("Generating Rotate...\n");
}

void counter()
{
    std::string startIndex;
    std::string stepString;
    printf("Start Index?\n");
    std::cin >> startIndex;
    printf("Number of Steps?\n");
    std::cin >> stepString;

    const int32_t start = atoi(startIndex.c_str());
    const int32_t steps = atoi(stepString.c_str());

    char buffer[64];
    int32_t status = 0;
    for(int32_t i=1; i<steps+1; i++)
    {
        memset(buffer, '\0', 64);
        sprintf(buffer, "/tmp/%d.bmp", start-i);
        std::string temp(buffer);
        gList.push_back(temp);
    }

    printf("Generating Counter...\n");
}

void fade()
{
    std::string from;
    std::string to;
    std::string stepString;
    std::string startIndex;
    printf("Fade from Image?\n");
    std::cin >> from;
    printf("Fade to Image?\n");
    std::cin >> to;
    printf("Number of Steps?\n");
    std::cin >> stepString;
    printf("Start Index?\n");
    std::cin >> startIndex;

    char buffer[1024];
    memset(buffer, '\0', 1024);
    sprintf(buffer, "cp %s /tmp/%s.bmp", from.c_str(), startIndex.c_str());
    int32_t status = system(buffer);
    assert(status == 0);
    memset(buffer, '\0', 1024);
    sprintf(buffer, "cp %s /tmp/%d.bmp", to.c_str(), atoi(startIndex.c_str()) + atoi(stepString.c_str()) + 1);
    status = system(buffer);
    assert(status == 0);

    char alias[64];
    memset(alias, '\0', 64);
    sprintf(alias, "/tmp/%d.bmp", atoi(startIndex.c_str()));
    gList.push_back(alias);

    const int32_t steps = atoi(stepString.c_str());
    const int32_t start = atoi(startIndex.c_str());

    int j = steps+1;
    for(int32_t i=start; i < start+steps; i++)
    {
        memset(buffer, '\0', 1024);
        sprintf(buffer, "convert /tmp/0.bmp /tmp/%d.bmp -evaluate-sequence mean /tmp/%d.bmp", j, j-1 );
        j--;

        status = system(buffer);
        assert(status == 0);
    }

    for(int32_t i=start+1; i < start+steps+2; i++)
    {
        memset(alias, '\0', 64);
        sprintf(alias, "/tmp/%d.bmp", i);
        std::string temp(alias);
        gList.push_back(temp);
    }

    printf("Generating Fade...\n");
}

void write()
{
    std::string name;
    std::string stage;
    std::string value;
    printf("Sequence Name?\n");
    std::cin >> name;
    printf("Stage Name?\n");
    std::cin >> stage;
    printf("Is Stage?\n");
    std::cin >> value;

    bool isStage = PARSE("T") || PARSE("t") || PARSE("True") || PARSE("true") || PARSE("y") || PARSE("yes") || PARSE("Y") || PARSE("Yes");

    char buffer[PATH_MAX];
    memset(buffer, '\0', PATH_MAX);
    sprintf(buffer, "%s.json", name.c_str());
    FILE* sequence = fopen(buffer, "w");
    assert(sequence);
    memset(buffer, '\0', PATH_MAX);
    sprintf(buffer, "{\n  \"name\": \"%s\",\n  \"stage\": \"%s\",\n", name.c_str(), stage.c_str());
    fwrite(buffer, 1, strlen(buffer), sequence);

    int32_t length = name.size();
    while(length--)
    {
        name[length] = toupper(name[length]);
    }

    std::string stageUpper(stage);
    if(isStage)
    {
        int32_t length = stage.size();
        while(length--)
        {
            stageUpper[length] = toupper(stage[length]);
        }
    }

    std::list<std::string> optimized;
    const int32_t count = gList.size();
    for(int32_t i = 0; i < count; i++)
    {
        memset(buffer, '\0', PATH_MAX);
        optimized.push_back(gList.front());
        const char* finalSlash = strrchr(gList.front().c_str(), '/');
        sprintf(buffer, "  \"sprite%d\": \"%s%d\"", i, isStage ? stageUpper.c_str(): name.c_str(), atoi(finalSlash+1));
        strcat(buffer, ",\n");
        gList.pop_front();
        fwrite(buffer, 1, strlen(buffer), sequence);
    }

    memset(buffer, '\0', PATH_MAX);
    sprintf(buffer, "  \"stage_only\": \"%s\"\n", isStage ? "true": "false");
    fwrite(buffer, 1, strlen(buffer), sequence);

    fwrite("}\n", 1, 2, sequence);
    fclose(sequence);

    optimized.sort();
    optimized.unique();
    FILE* manifest = fopen("manifest.json", "w");
    assert(manifest);

    std::string mixed(isStage ? stage: name);
    length = mixed.size();
    while(length-- > 1)
    {
        mixed[length] = tolower(mixed[length]);
    }
    mixed[length] = toupper(mixed[length]);

    printf("Writing sequence...\n");
    int32_t status = system("convert -delay 100 -loop 1 /tmp/*.bmp sequence.gif");
    assert(status == 0);

    while(optimized.size())
    {
        memset(buffer, '\0', PATH_MAX);
        const char* finalSlash = strrchr(optimized.front().c_str(), '/');
        int32_t j = atoi(finalSlash+1);
        sprintf(buffer, "  \"assets/%s%d.bmp\": \"%s%d\",\n", mixed.c_str(), j, isStage ? stageUpper.c_str(): name.c_str(), j);
        fwrite(buffer, 1, strlen(buffer), manifest);
        memset(buffer, '\0', PATH_MAX);
        sprintf(buffer, "mv /tmp/%d.bmp %s%d.bmp", j, mixed.c_str(), j);
        status = system(buffer);
        assert(status == 0);
        optimized.pop_front();
    }

    fclose(manifest);
}

int main(int argc, char** argv)
{
    int32_t status = system("rm /tmp/*.bmp");

    char* finalSlash = strrchr(argv[0], '/');
    std::string value;
    while(true)
    {
        printf("%s >> ", finalSlash+1);
        std::cin >> value;
        if(PARSE("rotate"))
        {
            rotate();
        }
        else if(PARSE("counter"))
        {
            counter();
        }
        else if(PARSE("fade"))
        {
            fade();
        }
        else if(PARSE("write"))
        {
            write();
        }
        else if(PARSE("exit"))
        {
            exit(0);
        }
        else
        {
            printf("Invalid command: Use Ctrl-C or type \"exit\" to end.\n");
        }
        printf("\n\n");
        value.clear();
    }

    return 0;
}
