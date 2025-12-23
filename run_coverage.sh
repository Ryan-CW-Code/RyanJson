#!/bin/bash
set -e  # 遇到错误立即退出

# ================================
# 1. 运行 fuzzer
# ================================
./build/linux/x86/release/RyanJson \
  ./test/fuzzer/corpus \
  -dict=./test/fuzzer/RyanJsonFuzzer.dict \
  -timeout=2 \
  -runs=999999 \
  -verbosity=0 \
  -max_len=16384

# ================================
# 2. 合并 profile 数据
# ================================
llvm-profdata merge -sparse default.profraw -o default.profdata

# ================================
# 3. 生成覆盖率报告（文本汇总）
# ================================
# 注意：llvm-cov report 只支持汇总统计，不支持行级参数
# --show-functions       显示函数级覆盖率
# --show-region-summary  显示区域覆盖率
llvm-cov report ./build/linux/x86/release/RyanJson \
  -instr-profile=default.profdata \
  -show-mcdc-summary \
  -show-functions \
  -show-region-summary \
  -sources ./test/fuzzer ./RyanJson

# ================================
# 4. 生成覆盖率报告（HTML详细）
# ================================
llvm-cov show ./build/linux/x86/release/RyanJson \
  -instr-profile=default.profdata \
  -format=html \
  -output-dir=docs \
  -show-mcdc-summary \
  -show-expansions \
  -show-regions \
  -show-line-counts-or-regions \
  -sources ./RyanJson
#   -sources ./test/fuzzer ./RyanJson
