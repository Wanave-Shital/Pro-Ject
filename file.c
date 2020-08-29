#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h> //for linux perpose
//#include<iostream>
//#include<io.h>//for windows
//malloc =allocate the memory on HARDDISK
#define MAXINODE 50//create a file,preprocessor,max-50files create on our project
#define READ 1
#define WRITE 2

#define MAXFILESIZE 2048  //2KB size allocate to memory

#define REGULAR 1 //types of file
#define SPECIAL 2

#define START 0 //file of offset
#define CURRENT 1
#define END 2


typedef struct superblock
{
    int TotalInodes;
    int FreeInode; //size=8byte
}SUPERBLOCK, *PSUPERBLOCK;//obj,ptr,superblock=contanis all file information 

typedef struct inode //actual used of ll in file system
{
    char FileName[50];
    int InodeNumber;
    int FileSize;//if we create a file allocate the some memory allocate some memory -10mb
    int FileActualSize;//2KB
    int FileType;//1=regular,2-speacial file
    char *Buffer;//character of ptr,used in malloc ,all opeations done on buffer
    int LinkCount;//always 1
    int ReferenceCount;//if file is open twice then increase the ref cnt by 2
    int permission; // 1 2 3
    struct inode *next;
}INODE,*PINODE,**PPINODE;
//self referntail structure-86*50 byte size
//not self strcute
typedef struct filetable
{
    int readoffset;//kotun read karne
    int writeoffset;//kote write kaarne
    int count;//always 1
    int mode; // 1-read, 2-write 3
    PINODE ptrinode;//point to inode
}FILETABLE,*PFILETABLE;


typedef struct ufdt//ekach member cha array banvaala aahe,karan prayat element mapping kartoy,to next pointer
{
    PFILETABLE ptrfiletable;//ptr pointer to filetable,filetable point to ptrinode,ptrinode point to itself
}UFDT;

UFDT UFDTArr[MAXINODE];//cteate a structure of array 0-49,GLOBAL

SUPERBLOCK SUPERBLOCKobj;//GLOBAL
PINODE head = NULL;//global ,storage class-extern,GLOBAL

void man(char *name)
{
    if(name == NULL) //ata name-create word aala aahe
     return;

    if(strcmp(name,"create") == 0)
    {
        printf("Description : Used to create new regular file\n");
        printf("Usage : create File_name Permission\n");
    }
    else if(strcmp(name,"read") == 0)
    {
        printf("Description : Used to read data from regular file\n");
        printf("Usage : read File_name No_Of_Bytes_To_Read\n");
    }
    else if(strcmp(name,"write") == 0)
    {
        printf("Description : Used to write into regular file\n");
        printf("Usage : write File_name\n After this enter the data that we want to write\n");
    }
    else if(strcmp(name,"ls") == 0)
    {
        printf("Description : Used to list all information of files\n");
        printf("Usage : ls\n");
    }
    else if(strcmp(name,"stat") == 0)
    {
        printf("Description : Used to display information of file\n");
        printf("Usage : stat File_name\n");
    }
    else if(strcmp(name,"fstat") == 0)
    {
        printf("Description : Used to display information of file\n");
        printf("Usage : stat File_Descriptor\n");
    }
    else if(strcmp(name,"truncate") == 0)
    {
        printf("Description : Used to remove data from file\n");
        printf("Usage : truncate File_name\n");
    }
    else if(strcmp(name,"open") == 0)
    {
        printf("Description : Used to open existing file\n");
        printf("Usage : open File_name mode\n");
    }
    else if(strcmp(name,"close") == 0)
    {
        printf("Description : Used to close opened file\n");
        printf("Usage : close File_name\n");
    }
    else if(strcmp(name,"closeall") == 0)
    {
        printf("Description : Used to close all opened file\n");
        printf("Usage : closeall\n");
    }
    else if(strcmp(name,"lseek") == 0)
    {

        printf("Description : Used to change file offset\n");
        printf("Usage : lseek File_Name ChangeInOffset StartPoint\n");
    }

    //lseek-reading the data from any keyword,movement of reading,wrin
    else if(strcmp(name,"rm") == 0)
    {
        printf("Description : Used to delete the file\n");
        printf("Usage : rm File_Name\n");
    }
    else
    {
        printf("ERROR : No manual entry available.\n");
    }
}

void DisplayHelp()
{
    printf("ls : To List out all files\n");
    printf("clear : To clear console\n");
    printf("open : To open the file\n");
    printf("close : To close the file\n");
    printf("closeall : To close all opened files\n");
    printf("read : To Read the contents from file\n");
    printf("write :To Write contents into file\n");
    printf("exit : To Terminate file system\n");
    printf("stat : To Display information of file using name\n");
    printf("fstat :To Display information of file using file descriptor\n");
    printf("truncate : To Remove all data from file\n");
    printf("rm : To Delet the file\n");
}

int GetFDFromName(char *name)
{
    int i = 0;

    while(i<MAXINODE)//50
    {
        if(UFDTArr[i].ptrfiletable != NULL)
            if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name)==0)
                break;
        i++;
    }
    if(i == MAXINODE) 
    return -1;

    else 
    return i;
}

PINODE Get_Inode(char * name)
{
    PINODE temp = head;
    int i = 0;

    if(name == NULL)
    return NULL;//file should be unique-NULL-SUCCESSFULLY UNIQUE FILE CREATED

    while(temp!= NULL)
    {
        if(strcmp(name,temp->FileName) == 0)
            break;
        temp = temp->next;
    }
    return temp;
}

void CreateDILB()//stater function
{
    int i = 1;//ll start with 1
    PINODE newn = NULL;
    PINODE temp = head;//copy value of head in temp

    while(i<= MAXINODE)//1-50//logic of InsetLast()
    {
        newn = (PINODE)malloc(sizeof(INODE));//86 byte allocated

        newn->LinkCount =0;
        newn->ReferenceCount = 0;
        newn->FileType = 0;
        newn->FileSize = 0;

        newn->Buffer = NULL;
        newn->next = NULL;

        newn->InodeNumber = i;

        if(temp == NULL)
        {
            head = newn;
            temp = head;
        }
        else
        {
            temp->next = newn;
            temp = temp->next;
        }
        i++;
    }
    printf("DILB created successfully\n");
}

void InitialiseSuperBlock()//starter Function
{
    int i = 0;
    while(i< MAXINODE)//loop iterate till 0-49
    {
        UFDTArr[i].ptrfiletable = NULL;
        i++;
    }

    SUPERBLOCKobj.TotalInodes = MAXINODE;//value are accessable bcoz its global
    SUPERBLOCKobj.FreeInode = MAXINODE;
}

int CreateFile(char *name,int permission)
{
    int i = 0;
    PINODE temp = head;//100 value
    // paramter wrong occur
    if((name == NULL) || (permission == 0) || (permission > 3))
    return -1;

    if(SUPERBLOCKobj.FreeInode == 0)

    return -2;

    (SUPERBLOCKobj.FreeInode)--;

    if(Get_Inode(name) != NULL)
    return -3;

    while(temp!= NULL)
    {
        if(temp->FileType == 0)
        break;
        temp=temp->next;
    }

    while(i<50)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = permission;
    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;
    UFDTArr[i].ptrfiletable->ptrinode = temp;

    strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name);
    UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
    UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
    UFDTArr[i].ptrfiletable->ptrinode->permission = permission;
    UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char *)malloc(MAXFILESIZE);
    return i;
}

// rm_File("Demo.txt")
int rm_File(char * name)
{
    int fd = 0;
    fd = GetFDFromName(name);//fd value=0
    if(fd == -1)
        return -1;

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;//1 to 0(for remove)

    if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;

        free(UFDTArr[fd].ptrfiletable->ptrinode->Buffer);
        strcpy(UFDTArr[fd].ptrfiletable->ptrinode->FileName,""); //memset
        UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount=0;
        UFDTArr[fd].ptrfiletable->ptrinode->permission=0;
        UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize=0;

        free(UFDTArr[fd].ptrfiletable);

    }

    UFDTArr[fd].ptrfiletable = NULL;
    (SUPERBLOCKobj.FreeInode)++;
}

int ReadFile(int fd, char *arr, int isize)//(0,70,3)
{
    int read_size = 0;

    if(UFDTArr[fd].ptrfiletable == NULL) 
    return -1;

    if(UFDTArr[fd].ptrfiletable->mode !=READ && UFDTArr[fd].ptrfiletable->mode !=READ+WRITE)
    return -2;

    if(UFDTArr[fd].ptrfiletable->ptrinode->permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->permission != READ+WRITE) 
    return -2;

    if(UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)//all the data read
    {
        return -3;
    }

    if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR) 
    {
        return -4;//janar nahi katar we used only regular file 
    }

    read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);//(4-0)
    if(read_size < isize)//4<3
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),read_size);//1000+0,3

        UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + read_size;//0+3
    }
    else
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),isize);

        (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + isize;
    }

    return isize;
}

int WriteFile(int fd, char *arr, int isize)//(0,baseadd=100(abcd),4)
{
    if(((UFDTArr[fd].ptrfiletable->mode) !=WRITE) && ((UFDTArr[fd].ptrfiletable->mode) !=READ+WRITE))
    { 
        return -1;
    }

    if(((UFDTArr[fd].ptrfiletable->ptrinode->permission) !=WRITE) && ((UFDTArr[fd].ptrfiletable->ptrinode->permission) != READ+WRITE))
    { 
        return -1;
    }
    if((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE) 
    {
        return -2;
    }

    if((UFDTArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR) 
    {
        return -3;
    }
    //strncpy=n no of bytes are copy
    if(((UFDTArr[fd].ptrfiletable->ptrinode->FileSize)-(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))< isize)
    {
        return -4;
    }

    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset),arr,isize);

    (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset )+ isize;

    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;

    return isize;
}

int OpenFile(char *name, int mode)
{
    int i = 0;
    PINODE temp = NULL;//100value

    if(name == NULL || mode <= 0)
        return -1;

    temp = Get_Inode(name);
    if(temp == NULL)
        return -2;

    if(temp->permission < mode)
        return -3;

    while(i<MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
        break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if(UFDTArr[i].ptrfiletable == NULL) 
    return -1;
    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = mode;
    if(mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    else if(mode == READ)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
    }
    else if(mode == WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    UFDTArr[i].ptrfiletable->ptrinode = temp;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

    return i;
}

/*void CloseFileByName(int fd)
{
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    (UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--; 
}*/

int CloseFileByName(char *name)
{
    int i = 0;
    i = GetFDFromName(name);//give fd of filename
    if(i == -1)
    return -1;

    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;

    return 0;
}

void CloseAllFile()//if referance  file more than 0 then (open file repeat) then reference count reset=0 
{
    int i = 0;
    while(i<MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            UFDTArr[i].ptrfiletable->readoffset = 0;
            UFDTArr[i].ptrfiletable->writeoffset = 0;
            //(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
            UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount=0;

        }
        i++;
    }
}
//lseek used for randomly read the data
//lseek(END=2) :-5 read the date from end side,read five data 
int LseekFile(int fd, int size, int from)//(fd=0,size=3,from=1(0/1/2))
{
    if((fd<0) || (from > 2)) 
        return -1;
    if(UFDTArr[fd].ptrfiletable == NULL) 
        return -1;

    if((UFDTArr[fd].ptrfiletable->mode == READ) || (UFDTArr[fd].ptrfiletable->mode == READ+WRITE))
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->readoffset) + size) > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) 
            return -1;

            if(((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0) 
            return -1;
            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + size;//0=3
        }
        else if(from == START)
        {
            if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)) 
            return -1;
            if(size < 0) 
            return -1;
            (UFDTArr[fd].ptrfiletable->readoffset) = size;
        }
        else if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
            return -1;

            if(((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0) 
            return -1;
            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
        }
    }

    else if(UFDTArr[fd].ptrfiletable->mode == WRITE)
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) > MAXFILESIZE) 
                return -1;
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0) 
                return -1;
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) =(UFDTArr[fd].ptrfiletable->writeoffset) + size;

            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
        }
        else if(from == START)
        {
            if(size > MAXFILESIZE) 
                return -1;
            if(size < 0) 
                return -1;
            if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = size;
            (UFDTArr[fd].ptrfiletable->writeoffset) = size;
        }
        else if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
                return -1;
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0) 
                return -1;
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
        }
    }
}

void ls_file()//logic of display
{
    int i = 0;
    PINODE temp = head;
    if(SUPERBLOCKobj.FreeInode == MAXINODE)//no any file
    {
        printf("Error : There are no files\n");
        return;
    }

    printf("\nFile Name\tInode number\tFile size\tLink count\n");
    printf("---------------------------------------------------------------\n");
    while(temp != NULL)
    {
        if(temp->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName,temp->InodeNumber,temp->FileActualSize,temp->LinkCount);
        }
        temp = temp->next;
    }
    printf("---------------------------------------------------------------\n");
}

int fstat_file(int fd)
{
    PINODE temp = head;
    int i = 0;

    if(fd < 0) 
        return -1;

    if(UFDTArr[fd].ptrfiletable == NULL) 
        return -2;
    if(temp->FileType==0)
    {
        return -2;
    }
    temp = UFDTArr[fd].ptrfiletable->ptrinode;
    //
    printf("\n---------Statistical Information about file----------\n");
    printf("File name : %s\n",temp->FileName);
    printf("Inode Number %d\n",temp->InodeNumber);
    printf("File size : %d\n",temp->FileSize);
    printf("Actual File size : %d\n",temp->FileActualSize);
    printf("Link count : %d\n",temp->LinkCount);
    printf("Reference count : %d\n",temp->ReferenceCount);

    if(temp->permission == 1)
        printf("File Permission : Read only\n");
    else if(temp->permission == 2)
        printf("File Permission : Write\n");
    else if(temp->permission == 3)
        printf("File Permission : Read & Write\n");
    printf("------------------------------------------------------\n\n");

    return 0;
}

int stat_file(char *name)
{
    PINODE temp = head;
    int i = 0;

    if(name == NULL) 
        return -1;

    //if(SUPERBLOCKobj.FreeInode==)
    while(temp!= NULL)
    {
        if(strcmp(name,temp->FileName) == 0)
            break;
        temp = temp->next;
    }

    if(temp == NULL)
    {
        return -2;
    }

    printf("\n---------Statistical Information about file----------\n");
    printf("File name : %s\n",temp->FileName);
    printf("Inode Number %d\n",temp->InodeNumber);
    printf("File size : %d\n",temp->FileSize);
    printf("Actual File size : %d\n",temp->FileActualSize);
    printf("Link count : %d\n",temp->LinkCount);
    printf("Reference count : %d\n",temp->ReferenceCount);

    if(temp->permission == 1)
        printf("File Permission : Read only\n");
    else if(temp->permission == 2)
        printf("File Permission : Write\n");
    else if(temp->permission == 3)
        printf("File Permission : Read & Write\n");
    printf("------------------------------------------------------\n\n");
    return 0;
}

int truncate_File(char *name)//file tich have asel pn tyatil data nko aahe yala truncate as as mahan
{
    int fd = GetFDFromName(name);
    if(fd == -1)
    return -1;

    memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,MAXFILESIZE);//buffer madhil data clean zala
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;
}
//Q-types of inode
int main()
{
    char *ptr = NULL;
    int ret = 0, fd = 0, count = 0;
    char command[4][80], str[80], arr[MAXFILESIZE];

    InitialiseSuperBlock();
    CreateDILB();

    while(1)
    {
        fflush(stdin);
        strcpy(str,"");

        printf("\nSheetal VFS :~$");

        fgets(str,80,stdin);// scanf("%[^'\n']s",str);

        count = sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);
        if(count == 1)
        {
            if(strcmp(command[0],"ls") == 0)
            {
                ls_file();
            }
            else if(strcmp(command[0],"closeall") == 0)
            {
                CloseAllFile();
                printf("All files closed successfully\n");
                continue;
            }
            else if(strcmp(command[0],"clear") == 0)
            {
                system("cls");
                continue;
            }
            else if(strcmp(command[0],"help") == 0)
            {
                DisplayHelp();
                continue;
            }
            else if(strcmp(command[0],"exit") == 0)
            {
                printf("Terminating the  Virtual File System\n");
                break;
            }
        else
        {
            printf("\nERROR : Command not found !!!\n");
            continue;
        }
    }
    else if(count == 2)
    {
        if(strcmp(command[0],"stat") == 0)
        {
            ret = stat_file(command[1]);
            if(ret == -1)
            
                printf("ERROR : Incorrect parameters\n");
            if(ret == -2)
                printf("ERROR : There is no such file\n");
            continue;
        }
        else if(strcmp(command[0],"fstat") == 0)
        {
            ret = fstat_file(atoi(command[1]));
            if(ret == -1)
                printf("ERROR : Incorrect parameters\n");
            if(ret == -2)
                printf("ERROR : There is no such file\n");
                continue;
        }
        else if(strcmp(command[0],"close") == 0)
        {
            ret = CloseFileByName(command[1]);
            if(ret == -1)
                printf("ERROR :sta There is no such file\n");
            continue;
        }
        else if(strcmp(command[0],"rm") == 0)//rm-remove
        {
            ret = rm_File(command[1]);
            if(ret == -1)
                printf("ERROR : There is no such file\n");
            continue;
        }
        else if(strcmp(command[0],"man") == 0)
        {
            man(command[1]);
        }
        else if(strcmp(command[0],"write") == 0)
        {
            fd = GetFDFromName(command[1]);
            if(fd == -1)
            {
                printf("Error : Incorrect parameter\n");
                continue;
            }

            fflush(stdin);

            printf("Enter the data : \n");
            scanf("%[^\n]",arr);

            ret = strlen(arr);//ret=5
            if(ret == 0)
            {
                printf("Error : Incorrect parameter\n");
                continue;
            }


            ret = WriteFile(fd,arr,ret);//(0,baseaddress(abcds),sizeofarray) fd=0
            if(ret == -1)
                printf("ERROR : Permission denied\n");
            if(ret == -2)
                printf("ERROR : There is no sufficient memory to write\n");
            if(ret == -3)
                printf("ERROR : It is not regular file\n");
            
        
            if(ret > 0)
            {
                printf("Success : %d Bytes Suceessfully written",ret);
            }
            if(ret==-4)
            {
                printf("Error:There is no sufficine tmemory available\n");
            }
        }

        else if(strcmp(command[0],"truncate") == 0)
        {
            ret = truncate_File(command[1]);
            /*if(ret == -1)
                printf("Error : Incorrect parameter\n");
                */
        }
        else
        {
            printf("\nERROR : Command not found !!!\n");
            continue;
        }
    }


    else if(count == 3)
    {
        if(strcmp(command[0],"create") == 0)
        {
            ret = CreateFile(command[1],atoi(command[2]));// atoi ascii to int
            if(ret >= 0)
                printf("File is successfully created with file descriptor : %d\n",ret);
            if(ret == -1)//create
                printf("ERROR : Incorrect parameters\n");
            if(ret == -2)
                printf("ERROR : There is no inodes\n");
            if(ret == -3)
                printf("ERROR : File already exists\n");
            if(ret == -4)
                printf("ERROR : Memory allocation failure\n");
            continue;
        }
        else if(strcmp(command[0],"open") == 0)
        {
            ret = OpenFile(command[1],atoi(command[2]));
            if(ret >= 0)
                printf("File is successfully opened with file descriptor : %d\n",ret);
            if(ret == -1)
                printf("ERROR : Incorrect parameters\n");
            if(ret == -2)
                printf("ERROR : File not present\n");
            if(ret == -3)
                printf("ERROR : Permission denied\n");
            continue;
        }  
    else if(strcmp(command[0],"read") == 0)
    {
        fd = GetFDFromName(command[1]);
        if(fd == -1)
        {
            printf("Error : Incorrect parameter\n");
            continue;
        }
        ptr = (char *)malloc(sizeof(atoi(command[2]))+1);
        if(ptr == NULL)
        {
            printf("Error : Memory allocation failure\n");
            continue;
        }
        ret = ReadFile(fd,ptr,atoi(command[2]));//(0,70(memoty) 3(for read))
        if(ret == -1)
            printf("ERROR : File not existing\n");
        if(ret == -2)
            printf("ERROR : Permission denied\n");
        if(ret == -3)
            printf("ERROR : Reached at end of file\n");
        if(ret == -4)
            printf("ERROR : It is not regular file\n");
        if(ret == 0)
            printf("ERROR : File empty\n");

        if(ret > 0)
        {
            write(2,ptr,ret);//like a printf,1-stdout,2-stdin,3-strerror
        }
        continue;
    }
        else
        {
            printf("\nERROR : Command not found !!!\n");
            continue;
        }
 }
    else if(count == 4)
    {
        if(strcmp(command[0],"lseek") == 0)
        {
            fd = GetFDFromName(command[1]);
            if(fd == -1)
            {
                printf("Error : Incorrect parameter\n");
                continue;
            }
            ret = LseekFile(fd,atoi(command[2]),atoi(command[3]));//(0,2(displacement),1(kutun pude jaycch,i,e CURRENT))
            //whole in file(gap is created(vacent space),(used only for writing the data) )-tyat suppose file size 1000 in that write first 300 bytes write then 
            //200 (keep vacent) start to 500 till the file

            //whole in the file(GAP) consired as potential 
            //swaping partision-part of harddisk it look like as RAM,create a set the enviorment for swapout(ram kadun harddisk var nene) the data. 
            //ram-magntism
            //harddisk-
            if(ret == -1)
            {
                printf("ERROR : Unable to perform lseek\n");
            }
        }
        else
        {
            printf("\nERROR : Command not found !!!\n");
            continue;
        }
    }
        else
        {
            printf("\nERROR : Command not found !!!\n");
            continue;
        }

    }
 return 0;
}


/*Commands:$
create Demo.txt 3
sheetal@sheetal-hp-laptop-15-da0xxx:~/PROJECT$ cc file.c
sheetal@sheetal-hp-laptop-15-da0xxx:~/PROJECT$ ./a.out
DILB created successfully

Sheetal VFS 4:~$man create
Description : Used to create new regular file
Usage : create File_name Permission

Sheetal VFS :~$ls
Error : There are no files

Sheetal VFS :~$create k.txt 3
File is successfully created with file descriptor : 0

Sheetal VFS :~$create k1.txt 3
File is successfully created with file descriptor : 1

Sheetal VFS :~$ls

File Name       Inode number    File size       Link count
---------------------------------------------------------------
k.txt           1               0               1
k1.txt          2               0               1
---------------------------------------------------------------

Sheetal VFS :~$cleat

ERROR : Command not found !!!

Sheetal VFS :~$clear
sh: 1: cls: not found

Sheetal VFS :~$stat k1.txt

---------Statistical Information about file----------
File name : k1.txt
Inode Number 2
File size : 2048
Actual File size : 0
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$fstat 0

---------Statistical Information about file----------
File name : k.txt
Inode Number 1
File size : 2048
Actual File size : 0
Link count : 1 //link count jar 0 zala tar ti delete zali file
Reference count : 1
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$stat abc.txt
ERROR : There is no such file

Sheetal VFS :~$stat k,txt 
ERROR : There is no such file

Sheetal VFS :~$stat k.txt

---------Statistical Information about file----------
File name : k.txt
Inode Number 1
File size : 2048
Actual File size : 0
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$create shital.txt 3
File is successfully created with file descriptor : 2

Sheetal VFS :~$fstat 4
ERROR : There is no such file

Sheetal VFS :~$fstat 2

---------Statistical Information about file----------
File name : shital.txt
Inode Number 3
File size : 2048
Actual File size : 0
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------
Sheetal VFS :~$close k1.txt //close 1(FD) internally takes a fd(UFDTArr of array index)

Sheetal VFS :~$close 1.txt //close file

Sheetal VFS :~$rm 2.txt //remove created file

Sheetal VFS :~$write 1.txt
Enter the data : 
asssddff

Sheetal VFS :~$
ERROR : Command not found !!!

Sheetal VFS :~$stat 1.txt

---------Statistical Information about file----------
File name : 1.txt
Inode Number 1
File size : 2048
Actual File size : 19
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------
Sheetal VFS :~$write 1.txt
Enter the data : 
asss
Success : 4 Bytes Suceessfully written

Sheetal VFS :~$read 1.txt 2
as
Sheetal VFS :~$read 1.txt 7
sssqww
Sheetal VFS :~$read 1.txt 3// 3 read=no of bytes

Sheetal VFS :~$man rm
Description : Used to delete the file
Usage : rm File_Name

Sheetal VFS :~$rm 2.txt
ERROR : There is no such file

Sheetal VFS :~$rm 1.txt

Sheetal VFS :~$ls


Sheetal VFS :~$



sheetal@sheetal-hp-laptop-15-da0xxx:~$ cc file.c
sheetal@sheetal-hp-laptop-15-da0xxx:~/PROJECT$ cc file.c
sheetal@sheetal-hp-laptop-15-da0xxx:~/PROJECT$ ./a.out
DILB created successfully

Sheetal VFS :~$man create
Description : Used to create new regular file
Usage : create File_name Permission

Sheetal VFS :~$create Demo.txt 3
File is successfully created with file descriptor : 0

Sheetal VFS :~$create Demo.txt 3
ERROR : File already exists

Sheetal VFS :~$ls     

File Name       Inode number    File size       Link count
---------------------------------------------------------------
Demo.txt                1               0               1
---------------------------------------------------------------

Sheetal VFS :~$create hello.txt 3
File is successfully created with file descriptor : 1

Sheetal VFS :~$ls

File Name       Inode number    File size       Link count
---------------------------------------------------------------
Demo.txt                1               0               1
hello.txt               2               0               1
---------------------------------------------------------------

Sheetal VFS :~$stat hello.txt

---------Statistical Information about file----------
File name : hello.txt
Inode Number 2
File size : 2048
Actual File size : 0
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$clear
sh: 1: cls: not found

Sheetal VFS :~$ clear
sh: 1: cls: not found

Sheetal VFS :~$fstat 0

---------Statistical Information about file----------
File name : Demo.txt
Inode Number 1
File size : 2048
Actual File size : 0
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$stat hello.txt

---------Statistical Information about file----------
File name : hello.txt
Inode Number 2
File size : 2048
Actual File size : 0
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$clear
sh: 1: cls: not found

Sheetal VFS :~$fstat 1

---------Statistical Information about file----------
File name : hello.txt
Inode Number 2
File size : 2048
Actual File size : 0
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$ stat abc.txt           
ERROR : There is no such file

Sheetal VFS :~$create abc.txt 1
File is successfully created with file descriptor : 2

Sheetal VFS :~$ls

File Name       Inode number    File size       Link count
---------------------------------------------------------------
Demo.txt                1               0               1
hello.txt               2               0               1
abc.txt         3               0               1
---------------------------------------------------------------

Sheetal VFS :~$write Demo.txt
Enter the data : 
Hello
Success : 5 Bytes Suceessfully written
Sheetal VFS :~$
ERROR : Command not found !!!

Sheetal VFS :~$stat Demo.txt

---------Statistical Information about file----------
File name : Demo.txt
Inode Number 1
File size : 2048
Actual File size : 5
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$write Demo.txt
Enter the data : 
abcdef
Success : 6 Bytes Suceessfully written
Sheetal VFS :~$
ERROR : Command not found !!!

Sheetal VFS :~$read Demo.txt 5
Hello
Sheetal VFS :~$read Demo.txt 2
ab
Sheetal VFS :~$read read Demo.txt 1

ERROR : Command not found !!!

Sheetal VFS :~$man rm
Description : Used to delete the file
Usage : rm File_Name

Sheetal VFS :~$rm Demo.txt

Sheetal VFS :~$ls

File Name       Inode number    File size       Link count
---------------------------------------------------------------
hello.txt               2               0               1
abc.txt         3               0               1
---------------------------------------------------------------

Sheetal VFS :~$create Demo.txt 3
File is successfully created with file descriptor : 0

Sheetal VFS :~$ls    

File Name       Inode number    File size       Link count
---------------------------------------------------------------
Demo.txt                1               0               1
hello.txt               2               0               1
abc.txt         3               0               1
---------------------------------------------------------------

Sheetal VFS :~$write hello.txt 
Enter the data : 
abcd
Success : 4 Bytes Suceessfully written
Sheetal VFS :~$
ERROR : Command not found !!!

Sheetal VFS :~$clear
sh: 1: cls: not found

Sheetal VFS :~$cls

ERROR : Command not found !!!

Sheetal VFS :~$man truncate
Description : Used to remove data from file
Usage : truncate File_name

Sheetal VFS :~$truncate Hello.txt
Error : Incorrect parameter

Sheetal VFS :~$ls

File Name       Inode number    File size       Link count
---------------------------------------------------------------
Demo.txt                1               0               1
hello.txt               2               4               1
abc.txt         3               0               1
---------------------------------------------------------------

Sheetal VFS :~$read Hello.txt 3
Error : Incorrect parameter

Sheetal VFS :~$help                      
ls : To List out all files
clear : To clear console
open : To open the file
close : To close the file
closeall : To close all opened files
read : To Read the contents from file
write :To Write contents into file
exit : To Terminate file system
stat : To Display information of file using name
fstat :To Display information of file using file descriptor
truncate : To Remove all data from file
rm : To Delet the file

Sheetal VFS :~$man open
Description : Used to open existing file
Usage : open File_name mode

Sheetal VFS :~$open hello.txt 3
File is successfully opened with file descriptor : 3

Sheetal VFS :~$stat hello.txt

---------Statistical Information about file----------
File name : hello.txt
Inode Number 2
File size : 2048
Actual File size : 4
Link count : 1
Reference count : 2
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$open hello.txt 3
File is successfully opened with file descriptor : 4

Sheetal VFS :~$stat hello.txt

---------Statistical Information about file----------
File name : hello.txt
Inode Number 2
File size : 2048
Actual File size : 4
Link count : 1
Reference count : 3
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$man close
Description : Used to close opened file
Usage : close File_name

Sheetal VFS :~$close hello.txt 

Sheetal VFS :~$ls

File Name       Inode number    File size       Link count
---------------------------------------------------------------
Demo.txt                1               0               1
hello.txt               2               4               1
abc.txt         3               0               1
---------------------------------------------------------------

Sheetal VFS :~$stat hello.txt

---------Statistical Information about file----------
File name : hello.txt
Inode Number 2
File size : 2048
Actual File size : 4
Link count : 1
Reference count : 2
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$close hello.txt

Sheetal VFS :~$ls

File Name       Inode number    File size       Link count
---------------------------------------------------------------
Demo.txt                1               0               1
hello.txt               2               4               1
abc.txt         3               0               1
---------------------------------------------------------------

Sheetal VFS :~$stat hello.txt

---------Statistical Information about file----------
File name : hello.txt
Inode Number 2
File size : 2048
Actual File size : 4
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$close hello.txt

Sheetal VFS :~$ls

File Name       Inode number    File size       Link count
---------------------------------------------------------------
Demo.txt                1               0               1
hello.txt               2               4               1
abc.txt         3               0               1
---------------------------------------------------------------

Sheetal VFS :~$stat hello.txt

---------Statistical Information about file----------
File name : hello.txt
Inode Number 2
File size : 2048
Actual File size : 4
Link count : 1
Reference count : 0
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$stat demo.txt
ERROR : There is no such file

Sheetal VFS :~$stat hello.txt

---------Statistical Information about file----------
File name : hello.txt
Inode Number 2
File size : 2048
Actual File size : 4
Link count : 1
Reference count : 0
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$stat Demo.txt

---------Statistical Information about file----------
File name : Demo.txt
Inode Number 1
File size : 2048
Actual File size : 0
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$write Demo.txt
Enter the data : 
abcde
Success : 5 Bytes Suceessfully written
Sheetal VFS :~$
ERROR : Command not found !!!

Sheetal VFS :~$stat Demo.txt

---------Statistical Information about file----------
File name : Demo.txt
Inode Number 1
File size : 2048
Actual File size : 5
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$read Demo.txt 1
a
Sheetal VFS :~$stat Demo.txt

---------Statistical Information about file----------
File name : Demo.txt
Inode Number 1
File size : 2048
Actual File size : 5
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$man lseek
Description : Used to change file offset
Usage : lseek File_Name ChangeInOffset StartPoint

Sheetal VFS :~$lseek Demo.txt 2 1

Sheetal VFS :~$read Demo.txt 1
d
Sheetal VFS :~$stat Demo.txt

---------Statistical Information about file----------
File name : Demo.txt
Inode Number 1
File size : 2048
Actual File size : 5
Link count : 1
Reference count : 1
File Permission : Read & Write
------------------------------------------------------


Sheetal VFS :~$read Demo.txt 2
e
Sheetal VFS :~$read Demo.txt 4
ERROR : Reached at end of file

Sheetal VFS :~$






*/