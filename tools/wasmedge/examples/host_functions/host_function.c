#include <wasmedge/wasmedge.h>
#include <stdio.h>
#include <json-c/json_tokener.h>

WasmEdge_Result parse_json(void *Data, WasmEdge_MemoryInstanceContext *MemCxt,
                    const WasmEdge_Value *In, WasmEdge_Value *Out) {
    FILE *fp = fopen("test.json","r");
    char buffer[1024];
    fread(buffer, 1024, 1, fp);
    fclose(fp);

    char key[32]; 
    snprintf(key, sizeof(key), "%d", WasmEdge_ValueGetI32(In[0]));
    struct json_object *number;
    json_object* parsedJson = json_tokener_parse(buffer);
    json_object_object_get_ex(parsedJson, key, &number);
    Out[0] = WasmEdge_ValueGenI32(json_object_get_int(number));
    return WasmEdge_Result_Success;
}

int main() {
  /* Create the VM context. */
  WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(NULL, NULL);

  /* Create the import object. */
  WasmEdge_String ExportName = WasmEdge_StringCreateByCString("extern");
  WasmEdge_ImportObjectContext *ImpObj = WasmEdge_ImportObjectCreate(ExportName);
  enum WasmEdge_ValType ParamList[1] = { WasmEdge_ValType_I32 };
  enum WasmEdge_ValType ReturnList[1] = { WasmEdge_ValType_I32 };
  WasmEdge_FunctionTypeContext *HostFType = WasmEdge_FunctionTypeCreate(ParamList, 1, ReturnList, 1);
  WasmEdge_FunctionInstanceContext *HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, parse_json, NULL, 0);
  WasmEdge_FunctionTypeDelete(HostFType);
  WasmEdge_String HostFuncName = WasmEdge_StringCreateByCString("func-parse-json");
  WasmEdge_ImportObjectAddFunction(ImpObj, HostFuncName, HostFunc);
  WasmEdge_StringDelete(HostFuncName);

  WasmEdge_VMRegisterModuleFromImport(VMCxt, ImpObj);

  /* The parameters and returns arrays. */
  WasmEdge_Value Params[1] = { WasmEdge_ValueGenI32(1000) };
  WasmEdge_Value Returns[1];
  WasmEdge_String FuncName = WasmEdge_StringCreateByCString("parse-json");
  WasmEdge_Result Res = WasmEdge_VMRunWasmFromFile(VMCxt, "parse-json.wasm", FuncName, Params, 1, Returns, 1);
  if (WasmEdge_ResultOK(Res)) {
    printf("Got the result: %d\n", WasmEdge_ValueGetI32(Returns[0]));
  } else {
    printf("Error message: %s\n", WasmEdge_ResultGetMessage(Res));
  }

  /* Resources deallocations. */
  WasmEdge_VMDelete(VMCxt);
  WasmEdge_StringDelete(FuncName);
  WasmEdge_ImportObjectDelete(ImpObj);
  return 0;
}