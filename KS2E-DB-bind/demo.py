import os
import KS2E_Client
# import protocol

# 初始化客户端
KS2E_Client.init()

# 更新操作：传入字符串 `str_w` 和 `str_id`，及其长度
str_w = "example_keyword"
str_id = "id1234"
i_length = len(str_id)
result = KS2E_Client.update(str_w, str_id)

# 搜索操作：传入 ID 字符串及其长度
search_id = "id1234"
i_length_search = len(search_id)
Tid = KS2E_Client.trapdoor_id(search_id) 
print("Tid with L and Jid:")
for item in Tid:
    hex_str = ' '.join(f'{b:02x}' for b in item)
    print(hex_str)

# 解码操作
b_list = [
    b'hello',
    b'world',
    b'\x01\x02\x03',
    b'binary_data_example'
]
search_w = "keyword1"
getids = KS2E_Client.decresult_w(search_w, b_list)
print("Decoded IDs:")
for item in getids:
    hex_str = ' '.join(f'{b:02x}' for b in item)
    print(hex_str)

# 释放客户端
KS2E_Client.py_free()

# v2

# # 初始化客户端
# protocol.init()

# # 更新操作：传入字符串 `str_w` 和 `str_id`，及其长度
# str_w = "example_data"
# str_id = "example_id"
# i_length = len(str_id)
# result = protocol.update(str_w, str_id, i_length)

# # 搜索操作：传入 ID 字符串及其长度
# search_id = "example_id"
# i_length_search = len(search_id)
# result = protocol.search_id(search_id, i_length_search)

# # 输出结果
# print("Search result:", result)

# # 释放客户端
# protocol.py_free()
