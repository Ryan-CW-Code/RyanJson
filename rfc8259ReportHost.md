# RFC8259 测试报告（Host）

更新时间：2026-03-16 18:17:51

说明：
- 严格模式：关闭（允许重复 key）
- 插入策略：尾插
- 科学计数法：开启
- 测试集来源：test/data/rfc8259（共 319 例，已内嵌到 RFC8259 测试代码中）
- 统计规则：y_/n_ 用例按预期判定，i_ 用例默认计入通过；语义差异不影响通过率。
- Host/QEMU 均使用内嵌数据集，避免依赖文件系统。

## 统计

| 组件 | 通过 | 总数 | 通过率 |
| --- | --- | --- | --- |
| RyanJson | 319 | 319 | 100.00% |
| yyjson | 319 | 319 | 100.00% |
| cJSON | 302 | 319 | 94.67% |

## RFC 失败汇总

| 组件 | 失败数 |
| --- | --- |
| RyanJson | 0 |
| yyjson | 0 |
| cJSON | 17 |

## RFC 失败记录

总计 17 条：

| 来源 | 类型 | 用例 | 期望 | 实际 |
| --- | --- | --- | --- | --- |
| cJSON | 期望失败但成功 | n_multidigit_number_then_00.json | fail | success |
| cJSON | 期望失败但成功 | n_number_-01.json | fail | success |
| cJSON | 期望失败但成功 | n_number_-2..json | fail | success |
| cJSON | 期望失败但成功 | n_number_0.e1.json | fail | success |
| cJSON | 期望失败但成功 | n_number_2.e+3.json | fail | success |
| cJSON | 期望失败但成功 | n_number_2.e-3.json | fail | success |
| cJSON | 期望失败但成功 | n_number_2.e3.json | fail | success |
| cJSON | 期望失败但成功 | n_number_neg_int_starting_with_zero.json | fail | success |
| cJSON | 期望失败但成功 | n_number_neg_real_without_int_part.json | fail | success |
| cJSON | 期望失败但成功 | n_number_real_without_fractional_part.json | fail | success |
| cJSON | 期望失败但成功 | n_number_with_leading_zero.json | fail | success |
| cJSON | 期望失败但成功 | n_string_invalid_unicode_escape.json | fail | success |
| cJSON | 期望失败但成功 | n_string_unescaped_ctrl_char.json | fail | success |
| cJSON | 期望失败但成功 | n_string_unescaped_newline.json | fail | success |
| cJSON | 期望失败但成功 | n_string_unescaped_tab.json | fail | success |
| cJSON | 期望失败但成功 | n_structure_null-byte-outside-string.json | fail | success |
| cJSON | 期望失败但成功 | n_structure_whitespace_formfeed.json | fail | success |
