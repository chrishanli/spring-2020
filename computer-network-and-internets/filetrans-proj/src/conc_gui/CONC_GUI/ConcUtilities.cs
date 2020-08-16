using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CONC_GUI
{
    public static class ConcUtilities
    {
        /*将int型转为byte[]型*/
        public static byte[] ConvertHex(int vel)
        {
            int velocity = vel;
            byte[] hex = new byte[4];
            hex[0] = (byte)((velocity >> 24) & 0xff);   //先右移再与操作
            hex[1] = (byte)((velocity >> 16) & 0xff);
            hex[2] = (byte)((velocity >> 8) & 0xff);
            hex[3] = (byte)(velocity & 0xff);
            return hex;
        }

        // 取得一个buff对应的long
        public static long GetLong(byte[] buf)
        {
            long val = 0;
            for (int j = 0; j < 7; j++)
            {
                val += buf[j];
                val <<= 8;
            }
            return val;
        }

        // 取得一个buff对应的int32
        public static long GetInt32(byte[] buf)
        {
            long val = 0;
            for (int j = 0; j < 3; j++)
            {
                val += buf[j];
                val <<= 8;
            }
            return val;
        }


    }
}
