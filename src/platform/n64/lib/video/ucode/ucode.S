.data

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

.global ParrotVideoBackend_rsp_ucode
.global ParrotVideoBackend_rsp_ucode_size

ParrotVideoBackend_rsp_ucode: .incbin TOSTRING(UCODE_PATH)
ParrotVideoBackend_rsp_ucode_size: .long . - ParrotVideoBackend_rsp_ucode