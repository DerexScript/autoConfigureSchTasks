#include <windows.h>
#include <winInet.h>
#include <string.h> /*strlen, */
#include <stdio.h> /* printf, scanf, NULL */
#include <stdlib.h> /* malloc, calloc, exit, free */

#include "include/curl/curl.h"

#define INITIAL_BUFFER (MAX_PATH * 5)

int extractResources(int id, int type, char path[]) {
    HMODULE hModule = GetModuleHandleA(NULL);
    HRSRC res=FindResourceA(hModule, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type) );
    if(res==NULL) {
        switch (GetLastError()) {
        case 1814:
            MessageBoxA(NULL, (char *)"Nao foi possível encontrar recurso!\nCodigo de erro: 1814", "Error", MB_ICONERROR|MB_OK);
            break;
        case 1813:
            MessageBoxA(NULL, (char *)"Nao foi possível encontrar o tipo do recurso!\nCodigo de erro: 1813", "Error", MB_ICONERROR|MB_OK);
            break;
        default:
            MessageBoxA(NULL, (char *)"UNKNOW Error", "Error", MB_ICONERROR|MB_OK);
            break;
        }
        return 0;
    }
    int dwSize=SizeofResource(hModule, res);
    if(!dwSize) {
        MessageBoxA(NULL, (char *)"ERROR_INSUFFICIENT_BUFFER", "Error", MB_ICONERROR|MB_OK);
        return 0;
    }
    HGLOBAL hRes=LoadResource(hModule, res);
    if(!hRes) {
        MessageBoxA(NULL, (char *)"ERROR_GET_IDENTIFIER", "Error", MB_ICONERROR|MB_OK);
        return 0;
    }
    void *pRes = LockResource(hRes);
    if (pRes == NULL) {
        MessageBoxA(NULL, (char *)"ERROR_RECEIVE_POINTER", "Error", MB_ICONERROR|MB_OK);
        return 0;
    }
    HANDLE hFile = CreateFileA(path, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile==INVALID_HANDLE_VALUE) {
        MessageBoxA(NULL, (char *)"INVALID_HANDLE_VALUE", "Error", MB_ICONERROR|MB_OK);
        return 0;
    }
    HANDLE hFilemap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, dwSize, NULL);
    if (hFilemap == NULL) {
        MessageBoxA(NULL, (char *)"Error", "Error", MB_ICONERROR|MB_OK);
        return 0;
    }
    void* lpBaseAddress = MapViewOfFile(hFilemap, FILE_MAP_WRITE, 0, 0, 0);
    CopyMemory(lpBaseAddress, pRes, dwSize);
    UnmapViewOfFile(lpBaseAddress);
    CloseHandle(hFilemap);
    CloseHandle(hFile);
    return 1;
}

void myCreateProcess (LPCTSTR lpApplicationName, LPCTSTR lpCurrentDirectory, int wShow, char *args, int isCMD) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    if (wShow == 1)
        si.wShowWindow = SW_MINIMIZE;
    ZeroMemory(&pi, sizeof(pi));
    if (!CreateProcessA(lpApplicationName, args, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, lpCurrentDirectory, &si, &pi)) {
        MessageBoxA(NULL, "Erro ao criar processo", "Error", MB_ICONEXCLAMATION|MB_OK);
    } else if(isCMD) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

char *concatN(char *str, char *str2) {
    int c = 0;
    int c1 = 0;
    char *newSTR = (char*)malloc(sizeof (char) * 150);
    do {
        newSTR[c] = str[c];
        c++;
    } while(str[c] != '\0');
    do {
        newSTR[c] = str2[c1];
        c1++;
        c++;
    } while(str2[c1] != '\0');
    newSTR[c] = '\0';
    return newSTR;
}

size_t ReadFunc(char *buffer, size_t b, size_t memb, void *data) {
    int sizeFile=0;
    if(buffer==NULL) {
        return 0;
    }
    sizeFile+=memb;
    return fwrite(buffer, b,memb, (FILE*)data);
}

int downloadF(char *url, char tempFullPath[]) {
    CURL *curl;
    CURLcode res;
    FILE *file = fopen(tempFullPath, "wb");
    curl = curl_easy_init();
    curl_easy_setopt(curl,CURLOPT_URL, url);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, ReadFunc);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(file);
    if(res == CURLE_OK) {
        return 1;
    } else {
        printf("Download Scheme File Error : %i\n", res);
        return 0;
    }
}

int main(int argc, char *argv[]) {
    char tempPath[MAX_PATH];
    GetTempPathA(sizeof(tempPath), tempPath);
    char *cmdCommand = (char *) calloc(200, sizeof(char));
    char *xmlPath = (char *) calloc(200, sizeof(char));
    FILE *fov;

    if(cmdCommand == NULL) {
        printf("Falha na alocacao de memoria!\n");
        free(cmdCommand);
        system("pause");
        return 0;
    }
    if(xmlPath == NULL) {
        printf("Falha na alocacao de memoria!\n");
        free(xmlPath);
        system("pause");
        return 0;
    }

    //obtem o path completo do arquivo xml
    xmlPath = tempPath;
    xmlPath = concatN(xmlPath, (char *)"adjustDateAndTime.xml");
    xmlPath = (char *) realloc(xmlPath, sizeof(char)*strlen(xmlPath)+1);
    if(xmlPath == NULL) {
        printf("Falha na realocacao de memoria!\n");
        free(xmlPath);
        system("pause");
        return 0;
    }

    //checa se há conexão com a internet
    if(InternetCheckConnection("http://google.com/", 1, 0)) {
        //faz download do arquivo xml
        if (!downloadF((char *)"http://pected.000webhostapp.com/adjustDateAndTime.xml", xmlPath)) {
            //extrai o arquivo xml do proprio executavel
            printf("Extranindo Arquido De Esquema...\n");
            fov = fopen(xmlPath, "r");
            if (!fov) {
                if(!extractResources(3, 256, xmlPath)) {
                    printf("Erro Ao Fazer Download/Extrair O Esquema De Energia!\n");
                    system("pause");
                    return 0;
                }
            }
            fclose(fov);
        }
    } else {
        //extrai o arquivo de esquema do proprio executavel
        printf("Extranindo Arquido De Esquema...\n");
        fov = fopen(xmlPath, "r");
        if (!fov) {
            if(!extractResources(3, 256, xmlPath)) {
                printf("Erro Ao Fazer Download/Extrair O Esquema De Energia!\n");
                system("pause");
                return 0;
            }
        }
        fclose(fov);
    }

    //adiciona xml a lista de tarefas
    cmdCommand = (char *)"C:\\Windows\\System32\\cmd.exe /c schtasks /create /xml ";
    cmdCommand = concatN(cmdCommand, xmlPath);
    cmdCommand = concatN(cmdCommand, (char *)" /tn adjustDateAndTime");
    cmdCommand = (char *) realloc(cmdCommand, sizeof(char)*strlen(cmdCommand)+1);
    if(cmdCommand == NULL) {
        printf("Falha na realocacao de memoria\n");
        free(cmdCommand);
        system("pause");
        return 0;
    }
    myCreateProcess(NULL, NULL, 1, cmdCommand, 1);
    printf("%s\n", cmdCommand);

    //deleta o arquivo xml da pasta %temp%
    DeleteFileA(xmlPath);

    //libera memoria
    free(cmdCommand);
    free(xmlPath);
    return 0;
}
