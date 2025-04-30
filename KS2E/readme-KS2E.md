# KS2E复现

## 底层函数

- 库：GMssl代替Openssl
- H->SHA512
- F->HMAC->SHA256

## 变量定义

- keyword/id/op：vector<unsigned char>
- 

## 协议模块

- setup
  - ![image-20250326164337381](readme-KS2E.assets/image-20250326164337381.png)
- update
  - ![image-20250326164359070](readme-KS2E.assets/image-20250326164359070.png)
- sharetoken
  - ![image-20250326164424092](readme-KS2E.assets/image-20250326164424092.png)
- share
  - ![image-20250326164454240](readme-KS2E.assets/image-20250326164454240.png)
- search
  - ![image-20250326164515419](readme-KS2E.assets/image-20250326164515419.png)