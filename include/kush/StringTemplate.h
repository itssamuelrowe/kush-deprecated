// Saturday, June 06 2020

#ifndef STRING_TEMPLATE_H
#define STRING_TEMPLATE_H

#include <jtk/collection/list/ArrayList.h>
#include <jtk/collection/map/HashMap.h>

/* Optionally, a map template entry may be associated with a tag. This allows
 * a template to invoke another template.
 * [main.template]
 * ```
 * <.names>
 * <$>
 * </>
 * ```
 * [full_name.template]
 * ```
 * <.first_name> <.last_name>
 * ```
 *
 * We feed the renderer:
 * ```
 * {
 *  [
 *      {
 *          "$tag": "full_name"
 *          "first_name": "Samuel"
 *          "last_name": "Rowe"
 *      },
 *      {
 *          "$tag": "full_name"
 *          "first_name": "Joel"
 *          "last_name": "Rego"
 *      }
 *  ]
 * }
 * ```
 *
 * Without tags, the render would not know how to render an object.
 */

#define TEMPLATE_ENTRY_STRING 0
#define TEMPLATE_ENTRY_INTEGER 1
#define TEMPLATE_ENTRY_DECIMAL 2
#define TEMPLATE_ENTRY_LIST 3
#define TEMPLATE_ENTRY_MAP 4
#define TEMPLATE_ENTRY_CUSTOM 5

typedef struct TemplateEntry TemplateEntry;

struct TemplateEntry {
    uint8_t tag;
    TemplateEntry* parent;
    union {
        int32_t integer;
        double decimal;
        struct {
            int32_t length;
            uint8_t* bytes;
        } string;
        jtk_ArrayList_t* list;
        jtk_HashMap_t* map;
        void* custom;
    };
};

TemplateEntry* makeIntegerEntry(TemplateEntry* parent, int32_t value);
TemplateEntry* makeDecimalEntry(TemplateEntry* parent, double value);
TemplateEntry* makeStringEntry(TemplateEntry* parent, uint8_t* bytes, int32_t length);
TemplateEntry* makeListEntry(TemplateEntry* parent);
TemplateEntry* makeMapEntry(TemplateEntry* parent);
TemplateEntry* makeCustomEntry(TemplateEntry* parent, void* custom);

typedef uint8_t* (*RenderCustom)(void* custom, int32_t* length);
typedef void (*DestroyCustom)(void* custom, int32_t* length);

struct StringTemplate {
    TemplateEntry* root;
    RenderCustom renderCustom;
    DestroyCustom destroyCustom;
};

typedef struct StringTemplate StringTemplate;

StringTemplate* stringTemplateNew();
void stringTemplateDelete(StringTemplate* self);
void stringTemplateAdd_i(StringTemplate* self, const uint8_t* name, int32_t value);
void stringTemplateAdd_d(StringTemplate* self, const uint8_t* name, double value);
void stringTemplateAdd_s(StringTemplate* self, const uint8_t* name, const uint8_t* bytes);
void stringTemplateAdd_v(StringTemplate* self, const uint8_t* name, void* custom);
void stringTemplateAddEx_i(StringTemplate* self, const uint8_t* name, int32_t length, int32_t value);
void stringTemplateAddEx_d(StringTemplate* self, const uint8_t* name, int32_t length, double value);
void stringTemplateAddEx_s(StringTemplate* self, const uint8_t* name, int32_t length, const uint8_t* bytes, int32_t bytesSize);
void stringTemplateAddEx_v(StringTemplate* self, const uint8_t* name, int32_t length, void* custom);

void stringTemplateSet_i(StringTemplate* self, const uint8_t* name, int32_t value);
void stringTemplateSet_d(StringTemplate* self, const uint8_t* name, double value);
void stringTemplateSet_s(StringTemplate* self, const uint8_t* name, const uint8_t* bytes);
void stringTemplateSet_v(StringTemplate* self, const uint8_t* name, void* custom);
void stringTemplateSetEx_i(StringTemplate* self, const uint8_t* name, int32_t length, int32_t value);
void stringTemplateSetEx_d(StringTemplate* self, const uint8_t* name, int32_t length, double value);
void stringTemplateSetEx_s(StringTemplate* self, const uint8_t* name, int32_t length, const uint8_t* bytes, int32_t bytesSize);
void stringTemplateSetEx_v(StringTemplate* self, const uint8_t* name, int32_t length, void* custom);

#endif /* STRING_TEMPLATE_H */

/* [test.template]
 * {{ title }}
 * {{ @names }}
 * Hi, {{ $ }}!
 * {{ ... }}
 *
 * [main.c]
 * int main() {
 *     StringTemplate* self = stringTemplateNew();
 *     stringTemplateSet_s(self, "title", "Hello, world!");
 *     stringTemplateAdd_s(self, "names", "Samuel");
 *     stringTemplateAdd_s(self, "names", "Joel");
 *     uint8_t* result = stringTemplateRenderFile(self, "test.template");
 *     printf("%s\n", result);
 *     return 0;
 * }
 */