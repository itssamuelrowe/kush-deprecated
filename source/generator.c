#include <kush/generator.h>

void generateStructures(Generator* generator, Module* module) {
    int32_t structureCount = jtk_ArrayList_getSize(module->structures);
    int32_t j;
    for (j = 0; j < structureCount; j++) {
        Structure* structure = (Structure*)jtk_ArrayList_getValue(
            module->structures, j);
        structure->type->llvmType = LLVMStructCreateNamed(generator->context, structure->name);
        
        int32_t declarationCount = jtk_ArrayList_getSize(structure->declarations);
        int32_t totalVariables = 0;
        for (int i = 0; i < declarationCount; i++) {
            VariableDeclaration* declaration =
                (VariableDeclaration*)jtk_ArrayList_getValue(structure->declarations, i);
            totalVariables += jtk_ArrayList_getSize(declaration->variables);
        }

        LLVMTypeRef* llvmVariableTypes = allocate(LLVMTypeRef, totalVariables);
        for (int i = 0; i < declarationCount; i++) {
            VariableDeclaration* declaration =
                (VariableDeclaration*)jtk_ArrayList_getValue(structure->declarations, i);

            int32_t variableCount = jtk_ArrayList_getSize(declaration->variables);
            int32_t m = 0;
            for (int k = 0; k < variableCount; k++) {
                Variable* variable = (Variable*)jtk_ArrayList_getValue(declaration->variables, k);
                llvmVariableTypes[m++] = variable->type->llvmType;
            }
        }
        LLVMStructSetBody(structure->type->llvmType, llvmVariableTypes, totalVariables, false);
    }
}

void generateFunctions(Generator* generator) {
    
}

bool generateLLVM(Generator* generator, Module* module, const char* name) {
    LLVMContextRef context = LLVMContextCreate();
    LLVMModuleRef llvmModule = LLVMModuleCreateWithNameInContext(name, context);
	LLVMSetDataLayout(llvmModule, "");
	LLVMSetTarget(llvmModule, LLVMGetDefaultTargetTriple());

    generator->context = context;
    generator->module = llvmModule;
    generator->builder = LLVMCreateBuilder();

    generateStructures(generator, module);
    generateFunctions(generator);

	char *error = NULL;
	bool invalid = LLVMVerifyModule(generator->module, LLVMAbortProcessAction, &error);
	if (!invalid) {
		char* data = LLVMPrintModuleToString(generator->module);
		fprintf(generator->output, data);
		LLVMDisposeMessage(data);
	}
    else {
        fprintf(stderr, "[error] %s\n", error);
    }
    LLVMDisposeMessage(error);

    LLVMContextDispose(context);
	
	return !invalid;
}

void generateIR(Generator* generator, Module* module) {
    Compiler* compiler = generator->compiler;
    const uint8_t* path = (const uint8_t*)jtk_ArrayList_getValue(compiler->inputFiles,
        compiler->currentFileIndex);

    int pathSize = jtk_CString_getSize(path);
    uint8_t* sourceName = allocate(uint8_t, pathSize - 1);

    int32_t i;
    for (i = 0; i < pathSize - 4; i++) {
        sourceName[i] = path[i];
    }
    sourceName[i] = 'l';
    sourceName[i + 1] = 'l';
    sourceName[i + 2] = '\0';

    generator->output = fopen(sourceName, "w+");
    generateLLVM(generator, module, sourceName);

    fclose(generator->output);
    deallocate(sourceName);
}

Generator* newGenerator(Compiler* compiler) {
    Generator* generator = allocate(Generator, 1);
    generator->compiler = compiler;
    generator->scope = NULL;
    generator->function = NULL;
    return generator;
}

void deleteGenerator(Generator* generator) {
    deallocate(generator);
}
