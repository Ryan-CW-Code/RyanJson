#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <intrin.h>

#include "./RyanJson/RyanJson.h"
#include "./cJSON/cJSON.h"
#include "./valloc/valloc.h"

int RyanJsonMemoryFootprint(char *jsonstr)
{
    RyanJsonInitHooks(v_malloc, v_free, v_realloc);

    RyanJson_t json = RyanJsonParse(jsonstr);
    if (json == NULL)
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return 0;
    }

    int area = 0, use = 0;
    v_mcheck(&area, &use);

    RyanJsonDelete(json);
    return use;
}

int cJSONMemoryFootprint(char *jsonstr)
{
    cJSON_Hooks hooks = {
        .malloc_fn = v_malloc,
        .free_fn = v_free};
    cJSON_InitHooks(&hooks);

    cJSON *json = cJSON_Parse(jsonstr);
    if (json == NULL)
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return 0;
    }

    int area = 0, use = 0;
    v_mcheck(&area, &use);

    cJSON_Delete(json);
    return use;
}

int RyanJsonMemoryFootprintTest()
{
    char *jsonstr;
    printf("\r\n--------------------------- 混合类型json数据测试 --------------------------\r\n");
    jsonstr = "{\"item1\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]},\"item2\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]},\"item3\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]},\"item4\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}}";
    printf("json原始文本长度为 %d, 序列化后RyanJson内存占用: %d, cJSON内存占用: %d\r\n", strlen(jsonstr), RyanJsonMemoryFootprint(jsonstr), cJSONMemoryFootprint(jsonstr));

    printf("\r\n--------------------------- 纯对象json数据测试 --------------------------\r\n");
    jsonstr = "{\"message\":\"success感谢又拍云(upyun.com)提供CDN赞助\",\"status\":200,\"date\":\"20230822\",\"time\":\"2023-08-22 09:44:54\",\"cityInfo\":{\"city\":\"郑州市\",\"citykey\":\"101180101\",\"parent\":\"河南\",\"updateTime\":\"07:46\"},\"data\":{\"shidu\":\"85%\",\"pm25\":20,\"pm10\":56,\"quality\":\"良\",\"wendu\":\"29\",\"ganmao\":\"极少数敏感人群应减少户外活动\",\"forecast\":[{\"date\":\"22\",\"high\":\"高温 35℃\",\"low\":\"低温 23℃\",\"ymd\":\"2023-08-22\",\"week\":\"星期二\",\"sunrise\":\"05:51\",\"sunset\":\"19:05\",\"aqi\":78,\"fx\":\"东南风\",\"fl\":\"2级\",\"type\":\"晴\",\"notice\":\"愿你拥有比阳光明媚的心情\"},{\"date\":\"23\",\"high\":\"高温 33℃\",\"low\":\"低温 23℃\",\"ymd\":\"2023-08-23\",\"week\":\"星期三\",\"sunrise\":\"05:52\",\"sunset\":\"19:04\",\"aqi\":71,\"fx\":\"南风\",\"fl\":\"2级\",\"type\":\"中雨\",\"notice\":\"记得随身携带雨伞哦\"},{\"date\":\"24\",\"high\":\"高温 31℃\",\"low\":\"低温 21℃\",\"ymd\":\"2023-08-24\",\"week\":\"星期四\",\"sunrise\":\"05:52\",\"sunset\":\"19:03\",\"aqi\":74,\"fx\":\"东风\",\"fl\":\"2级\",\"type\":\"晴\",\"notice\":\"愿你拥有比阳光明媚的心情\"},{\"date\":\"25\",\"high\":\"高温 30℃\",\"low\":\"低温 23℃\",\"ymd\":\"2023-08-25\",\"week\":\"星期五\",\"sunrise\":\"05:53\",\"sunset\":\"19:02\",\"aqi\":93,\"fx\":\"东风\",\"fl\":\"1级\",\"type\":\"小雨\",\"notice\":\"雨虽小，注意保暖别感冒\"},{\"date\":\"26\",\"high\":\"高温 25℃\",\"low\":\"低温 22℃\",\"ymd\":\"2023-08-26\",\"week\":\"星期六\",\"sunrise\":\"05:54\",\"sunset\":\"19:00\",\"aqi\":80,\"fx\":\"东北风\",\"fl\":\"1级\",\"type\":\"阴\",\"notice\":\"不要被阴云遮挡住好心情\"},{\"date\":\"27\",\"high\":\"高温 27℃\",\"low\":\"低温 20℃\",\"ymd\":\"2023-08-27\",\"week\":\"星期日\",\"sunrise\":\"05:55\",\"sunset\":\"18:59\",\"aqi\":74,\"fx\":\"西北风\",\"fl\":\"1级\",\"type\":\"阴\",\"notice\":\"不要被阴云遮挡住好心情\"},{\"date\":\"28\",\"high\":\"高温 30℃\",\"low\":\"低温 20℃\",\"ymd\":\"2023-08-28\",\"week\":\"星期 一\",\"sunrise\":\"05:55\",\"sunset\":\"18:58\",\"aqi\":80,\"fx\":\"东北风\",\"fl\":\"2级\",\"type\":\"多云\",\"notice\":\"阴晴之间，谨防紫外线侵扰\"},{\"date\":\"29\",\"high\":\"高温 30℃\",\"low\":\"低温 20℃\",\"ymd\":\"2023-08-29\",\"week\":\"星期二\",\"sunrise\":\"05:56\",\"sunset\":\"18:56\",\"aqi\":80,\"fx\":\"东北风\",\"fl\":\"2级\",\"type\":\"多云\",\"notice\":\"阴晴之间，谨防紫外线侵扰\"},{\"date\":\"30\",\"high\":\"高温 31℃\",\"low\":\"低温 20℃\",\"ymd\":\"2023-08-30\",\"week\":\"星期三\",\"sunrise\":\"05:57\",\"sunset\":\"18:55\",\"aqi\":92,\"fx\":\"南风\",\"fl\":\"1级\",\"type\":\"晴\",\"notice\":\"愿你拥有比阳光明媚的心情\"},{\"date\":\"31\",\"high\":\"高温 33℃\",\"low\":\" 低温 22℃\",\"ymd\":\"2023-08-31\",\"week\":\"星期四\",\"sunrise\":\"05:57\",\"sunset\":\"18:54\",\"aqi\":91,\"fx\":\"南风\",\"fl\":\"1级\",\"type\":\"晴\",\"notice\":\"愿你拥有比阳光明媚的心情\"},{\"date\":\"01\",\"high\":\"高温 34℃\",\"low\":\"低温 23℃\",\"ymd\":\"2023-09-01\",\"week\":\"星期五\",\"sunrise\":\"05:58\",\"sunset\":\"18:52\",\"aqi\":91,\"fx\":\"西风\",\"fl\":\"1级\",\"type\":\"晴\",\"notice\":\"愿你拥有比阳光明媚的心情\"},{\"date\":\"02\",\"high\":\"高温 36℃\",\"low\":\"低温 25℃\",\"ymd\":\"2023-09-02\",\"week\":\"星期六\",\"sunrise\":\"05:59\",\"sunset\":\"18:51\",\"aqi\":78,\"fx\":\"南风\",\"fl\":\"1级\",\"type\":\"阴\",\"notice\":\"不要被阴云遮挡住好心情\"},{\"date\":\"03\",\"high\":\"高温 35℃\",\"low\":\"低温 24℃\",\"ymd\":\"2023-09-03\",\"week\":\"星期日\",\"sunrise\":\"06:00\",\"sunset\":\"18:50\",\"aqi\":82,\"fx\":\"东北风\",\"fl\":\"1级\",\"type\":\"晴\",\"notice\":\"愿你拥有比阳光明媚的心情\"},{\"date\":\"04\",\"high\":\"高温 35℃\",\"low\":\"低温 25℃\",\"ymd\":\"2023-09-04\",\"week\":\"星期一\",\"sunrise\":\"06:00\",\"sunset\":\"18:48\",\"aqi\":88,\"fx\":\"南风\",\"fl\":\"2级\",\"type\":\"晴\",\"notice\":\"愿你拥有比阳光明媚的心情\"},{\"date\":\"05\",\"high\":\"高温 35℃\",\"low\":\"低温 25℃\",\"ymd\":\"2023-09-05\",\"week\":\"星期二\",\"sunrise\":\"06:01\",\"sunset\":\"18:47\",\"aqi\":58,\"fx\":\"南风\",\"fl\":\"2级\",\"type\":\"阴\",\"notice\":\"不要被阴云遮挡住好心情\"}],\"yesterday\":{\"date\":\"21\",\"high\":\"高温 34℃\",\"low\":\"低温 24℃\",\"ymd\":\"2023-08-21\",\"week\":\" 星期一\",\"sunrise\":\"05:50\",\"sunset\":\"19:07\",\"aqi\":60,\"fx\":\"西风\",\"fl\":\"2级\",\"type\":\"小雨\",\"notice\":\"雨虽小，注意保暖别感冒\"}}}";
    printf("json原始文本长度为 %d, 序列化后RyanJson内存占用: %d, cJSON内存占用: %d\r\n", strlen(jsonstr), RyanJsonMemoryFootprint(jsonstr), cJSONMemoryFootprint(jsonstr));

    printf("\r\n--------------------------- 小对象json 混合类型内存占用测试 --------------------------\r\n");
    jsonstr = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}";
    printf("json原始文本长度为 %d, 序列化后RyanJson内存占用: %d, cJSON内存占用: %d\r\n", strlen(jsonstr), RyanJsonMemoryFootprint(jsonstr), cJSONMemoryFootprint(jsonstr));

    printf("\r\n--------------------------- 小对象json 纯字符串内存占用测试 --------------------------\r\n");
    jsonstr = "{\"inter\":\"16\",\"double\":\"16.89\",\"string\":\"hello\",\"boolTrue\":\"true\",\"boolFalse\":\"false\",\"null\":\"null\"}";
    printf("json原始文本长度为 %d, 序列化后RyanJson内存占用: %d, cJSON内存占用: %d\r\n", strlen(jsonstr), RyanJsonMemoryFootprint(jsonstr), cJSONMemoryFootprint(jsonstr));

    /**
     * @brief 反序列化为文本，内存占用没什么特别的优化点，和cjson实现思路差不多，内存占用也就差不多，就不进行对比了
     *
     */

    return 1;
}