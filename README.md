# RyanJson
***一个针对资源受限的嵌入式设备优化的Json库，内存占用极小的通用json库，简洁高效！***

*初衷：项目进行重构json结构变复杂了很多，cJSON内存占用太高，已经满足不了需求。*

## 1、介绍

RyanJson是一个小巧的c语言json解析器，包含json文本文件解析 / 生成，**专门针对内存占用进行优化**。

相比cJSON内存占用减少30% - 60%，运行速度和cJSON差不多，在json文本数据小于80k / 嵌套深度3000以下的json数据，RyanJson和cJSON因为小巧的设计速度要比别的json库快得多。

- **低内存占用**：使用动态扩展技术，在32位系统下，一个基础json节点仅占用8字节。
- **开发人员友好**：仅有一个c文件和头文件轻松集成，hook函数方便自定义内存钩子。类cJSON的api，迁移成本低。
- **严格但不严苛**：符合 RFC 8295 大部分JSON标准，支持无限的json嵌套级别（需注意堆栈空间）、灵活的配置修改项
- **可扩展性**：允许注释(需调用mini函数清除注释后再解析)、尾随逗号等无效字符(parse时可配置是否允许)等

## 2、局限性

- 使用int / double表示json中的number类型，精度有所丢失。建议64位的number类型最好用string字符串表示。
- 对象中允许有重复的key，RyanJson库采用单向链表，会访问到第一个对象。

## 3、测试

测试代码可在本项目根目录查看。

#### 内存占用测试

![image-20230822200726742](docs/assert/README.assert/image-20230822200726742.png)

#### RFC 8295 标准测试，大部分嵌入式场景不会出现复杂的特殊json结构

***RyanJson和cJSON都不适合处理复杂的UTF-16字符集，如果项目需要兼容Unicode字符集，可以考虑yyjson / json-c***

![image-20230822200717809](docs/assert/README.assert/image-20230822200717809.png)

## 4、文档

文档可在 [Ryan文档中心](https://ryan-cw-code.github.io/RyanDocs/)获取

## 5、联系

Email：1831931681@qq.com

wx：17513216936
