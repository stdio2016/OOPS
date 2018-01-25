所有物件都是繼承自 `void`

`null` 是特殊物件，在這個物件上呼叫的任何程序都不會執行，而且會傳回 `null`

`void` 有以下程序：

* `void getchar()`

  從標準輸入讀取 1 個位元，從高位元到低位元

  如果是 0 或檔案結尾，則傳回 `null`，如果是 1 則傳回自己

* `void feof()`

  如果標準輸入已到達檔案結尾，則傳回自己，否則傳回 `null`

* `void puts(void str)`

  印出字串 `str`，注意字串結尾不會換行

  如果 `str` 不是字串，則會印出 `str` 物件的結構

  傳回值永遠是 `null`

* `void putchar(void bit)`

  輸出一個位元到標準輸出

  如果 bit 是 `null`，則輸出 0，否則輸出 1

  傳回值永遠是 `null`
