#include <stdio.h>
#include <kush/StringTemplate.h>

int main() {
    StringTemplate* self = stringTemplateNew();
    stringTemplateSet_s(self, "title", "Hello, world!");
    stringTemplateAdd_s(self, "names", "Samuel Rowe");
    stringTemplateAdd_s(self, "names", "Joel E. Rego");

    uint8_t* result = stringTemplateRenderFile(self, "test");
    printf("%s\n", result);

    free(result);
    stringTemplateDelete(result);

    return 0;
}