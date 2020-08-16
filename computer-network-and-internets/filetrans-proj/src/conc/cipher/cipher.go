package cipher

import (
	"bytes"
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"encoding/pem"
	"errors"
	"math/big"
	"os"
	"path/filepath"
)

// 生成RSA公私钥（输入：密钥的比特数、保存密钥至的目录）
func GenRSAKeysFiles(bits int, saveTo string) error {
	// 看看这个目录是否存在
	stat, err := os.Stat(saveTo)
	if err != nil { return err }
	if !stat.IsDir() { return errors.New("destination is not a directory") }
	// 生成公私钥
	privateKey, err := rsa.GenerateKey(rand.Reader, bits)
	if err != nil {
		return err
	}
	// 保存私钥（pem x509格式）
	derStream := x509.MarshalPKCS1PrivateKey(privateKey)
	block := &pem.Block {
		Type:  "RSA PRIVATE KEY",
		Bytes: derStream,
	}
	keyPath := filepath.Join(saveTo, ".conc~private.pem")
	file, err := os.Create(keyPath)
	if err != nil {
		return err
	}
	err = pem.Encode(file, block)
	if err != nil {
		return err
	}
	// 保存公钥（pem x509）
	publicKey := &privateKey.PublicKey
	derPkix, err := x509.MarshalPKIXPublicKey(publicKey)
	if err != nil {
		return err
	}
	block = &pem.Block{
		Type:  "PUBLIC KEY",
		Bytes: derPkix,
	}
	keyPath = filepath.Join(saveTo, ".conc~public.pem")
	file, err = os.Create(keyPath)
	if err != nil {
		return err
	}
	err = pem.Encode(file, block)
	if err != nil {
		return err
	}
	return nil
}

// 生成RSA公私钥（输入：密钥的比特数、保存密钥至的目录）
func GenRSAKeys(bits int) (pubKey []byte, priKey []byte, err error) {
	keys, err := rsa.GenerateKey(rand.Reader, bits)
	if err != nil {
		return nil, nil, err
	}
	// 保存私钥（pem x509格式）
	derStream := x509.MarshalPKCS1PrivateKey(keys)
	block := &pem.Block {
		Type:  "RSA PRIVATE KEY",
		Bytes: derStream,
	}
	priKey = pem.EncodeToMemory(block)
	// 保存公钥（pem x509）
	publicKey := &keys.PublicKey
	derPkix, err := x509.MarshalPKIXPublicKey(publicKey)
	if err != nil {
		return nil, nil, err
	}
	block = &pem.Block{
		Type:  "PUBLIC KEY",
		Bytes: derPkix,
	}
	pubKey = pem.EncodeToMemory(block)
	if pubKey == nil || priKey == nil {
		return nil, nil, errors.New("generate keys error")
	}
	return pubKey, priKey, err
}

// 删除RSA公私钥（输入：公私钥存储的目录）
func DelRESKeys(from string) error {
	// 看看这个目录是否存在
	stat, err := os.Stat(from)
	if err != nil { return err }
	if !stat.IsDir() { return errors.New("destination is not a directory")}
	// 尝试删除（尝试而已，忽略错误）
	err = os.Remove(filepath.Join(from, ".conc~public.pem"))
	err = os.Remove(filepath.Join(from, ".conc~private.pem"))
	return nil
}

// RSA加密（输入：公钥文件存储的目录、明文，输出：密文）
func RSAEncrypt(plainText []byte, pubKey []byte) (encoded []byte) {
	// pem及x509解码
	block, _ := pem.Decode(pubKey)
	publicKeyInterface, err := x509.ParsePKIXPublicKey(block.Bytes)
	if err != nil {
		panic(err)
	}
	// 类型断言
	publicKey := publicKeyInterface.(*rsa.PublicKey)
	// 对明文进行加密
	cipherText, err := rsa.EncryptPKCS1v15(rand.Reader, publicKey, plainText)
	if err!=nil{
		panic(err)
	}
	//返回密文
	return cipherText
}

// RSA解密（输入：私钥文件存储的目录、密文，输出：明文）
func RSADecrypt(encodedBits []byte, priKey []byte) (decoded []byte) {
	// pem及X509解码解码
	block, _ := pem.Decode(priKey)
	privateKey, err := x509.ParsePKCS1PrivateKey(block.Bytes)
	if err!=nil{
		panic(err)
	}
	// 对密文进行解密
	plainText,_:=rsa.DecryptPKCS1v15(rand.Reader, privateKey, encodedBits)
	// 返回明文
	return plainText
}

// AES解密
func AESDecrypt(orig, key[] byte) []byte {
	block, _ := aes.NewCipher(key)
	blockSize := block.BlockSize()
	blockMode := cipher.NewCBCDecrypter(block, key[:blockSize])
	origData := make([]byte, len(orig))
	blockMode.CryptBlocks(origData, orig)
	// 去补码（AES解密用）
	unPadding := int(origData[len(origData) - 1])
	return origData[:len(origData) - unPadding]
}

// AES加密
func AESEncrypt(origData, key []byte) []byte {
	// 新建加密器及补码
	block, _ :=aes.NewCipher(key)
	blockSize := block.BlockSize()
	// 计算需要补几位数
	padding := blockSize - len(origData) % blockSize
	//fmt.Println(padding)
	// 在切片后面追加char数量的byte(char) 	切片后面的第一个字符是增加的padding数量
	origData = append(origData, bytes.Repeat([]byte { byte(padding) }, padding)...)
	// 设置加密格式
	blockMode := cipher.NewCBCEncrypter(block,key[:blockSize])
	// 创建缓冲区
	encrypted := make([]byte, len(origData))
	// 开始加密
	blockMode.CryptBlocks(encrypted, origData)
	return encrypted
}

	// 生成随机的16位密码
func GenAESKey() (key []byte) {
	key = make([]byte, 16)
	for i := 0; i < 16; i++  {
		n, _ := rand.Int(rand.Reader, big.NewInt(255))
		key[i] = byte(n.Int64())
	}
	return key
}
