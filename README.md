# Graphics2D

## 新版本特性

不支持`G2dObject`数据结构，改用`G2D_OBJECT`

`G2D_OBJECT`支持隐式转换为`ID2D1Factory`, `ID2D1RenderTarget`, `IDWriteFactory5`, `HWND`数据结构

新版本需要手动初始化G2D库，并且初始化WIC之前需要手动初始化COM

更改了加载字体的方式，需要将字体数据加载到`G2D_FONTCOLLECTION`

`G2D_FONTCOLLECTION`支持隐式转化为`IDWriteFontCollection1`数据结构

释放字体数据时请使用`G2D_FONTCOLLECTION`的`Release`方法，这可以同时释放由G2D内部创建的资源（该部分资源对您不可见），否则可能会导致内存泄漏
