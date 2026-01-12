#!/bin/bash
set -e  # 遇到错误立即退出

xmake
echo "xmake build success"

# ================================
# 1. 运行
# ================================
./build/linux/x86/release/RyanJson


# ================================
# 2. 合并 profile 数据
# ================================
llvm-profdata merge -sparse default.profraw -o default.profdata

# ================================
# 3. 生成覆盖率报告（文本汇总）
# ================================
# 注意：llvm-cov report 只支持汇总统计，不支持行级参数
llvm-cov report ./build/linux/x86/release/RyanJson \
  -instr-profile=default.profdata \
  -show-mcdc-summary \
  -sources ./RyanJson

# ================================
# 4. 生成覆盖率报告（HTML详细）
# ================================
llvm-cov show ./build/linux/x86/release/RyanJson \
  -instr-profile=default.profdata \
  -format=html \
  -output-dir=coverage/docs \
  -sources ./RyanJson
#   -sources ./test/fuzzer ./RyanJson
