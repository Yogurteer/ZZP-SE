from setuptools import setup, Extension
from setuptools.command.build import build

class CustomBuildCommand(build):
    def run(self):
        # 修改构建路径为当前目录
        self.build_lib = './mybuild'
        super().run()

module = Extension(
    'KS2E_Client',  # Python 导入时使用的模块名
    sources=[
        'i_client.cpp',
        'utils/utils.cpp',
        'utils/encrypt.cpp',
        'utils/inter.cpp',
        'include/KS2E.cpp'
    ],  # 源代码文件
    include_dirs=[
        './include', 
        './utils',
        '/root/miniconda3/envs/cenv/include',
        '/usr/include'
    ], 
    extra_compile_args=['-std=c++17'],  # 如果需要额外的编译选项
    libraries=['ssl', 'crypto', 'pq'],  # 链接 OpenSSL 的 ssl 和 crypto 库
    library_dirs=[
        '/root/miniconda3/envs/cenv/lib',  # OpenSSL 库路径
        '/usr/lib/x86_64-linux-gnu',       # 如果 libpq 库位于这里，可以加入
        '/usr/local/pgsql/lib',            # PostgreSQL 默认库路径
        '/usr/lib'                   # 如果你有特定的 libpq 库路径，请修改这里
    ],  # 添加 OpenSSL 库的目录
)

setup(
    name='KS2E_Client',
    version='1.0',
    description='KS2E Client for Python',
    ext_modules=[module],
    cmdclass={'build': CustomBuildCommand},  # 使用自定义的构建命令
)
