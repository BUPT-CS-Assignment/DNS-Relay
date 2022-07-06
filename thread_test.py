# encoding=utf-8

import datetime
import os
import threading

def exec(cmd):
    try:
        print("exec:%s time:%s"%(cmd,datetime.datetime.now()));
        os.system(cmd);
    except:
        print("exec failed.");



if __name__ == "__main__":
    name_list=['bupt.edu.cn','baidu.com','bing.com','unique.com','noui.cloud','test.com'];
    while(1):
        threads=[];
        for item in name_list:
            
            th = threading.Thread(target=exec,args=("nslookup "+item+ " 127.0.0.1",))
            th.start()
            threads.append(th)

        for th in threads:
            th.join()
        
        os.system('pause');
