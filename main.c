#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int N = 64;
int T = 16;

//2^4, 2^6, 2^8
FILE *file;



// Key Scheduling Algorithm
void ksa(unsigned char state[], unsigned char key[])
{
    int i,j=0,t;
    int len = strlen((char*)key);
    for (i=0; i < N; ++i)
        state[i] = i;
    for (i=0; i < T; ++i) {
        j = (j + state[i] + key[i % len]) % N;
        t = state[i];
        state[i] = state[j];
        state[j] = t;
    }
}


int bitOfKey(unsigned char key[], int bit){
    
    int n_byte = bit / 8; // numer byte'a
    bit -= n_byte *8;       // numer bitu w konkretnym byte'ie
    
    unsigned char byte = key[n_byte];
    
    bit = byte >> (8 - bit-1); // shift, so last bit will be desired bit
    
    if (bit % 2 == 0){
        return 0;
    }else{
        return 1;
    }
    
}


void ksa_rs(unsigned char state[], unsigned char key[])
{
    int i, r;
    int top[256], bottom[256];
    int top_ctr = 0, bottom_ctr = 0;
    int len = strlen((char*)key);
    
    unsigned char newState[256];
    
    for (i=0; i < N; ++i)
        state[i] = i;
    
    for (r=0; r < T; ++r) {
        
        
        top_ctr = 0;
        bottom_ctr = 0;
        
        
        for (i=0; i < N; ++i){
            if (bitOfKey(key, (r*N + i) % (len*8)) == 0){
                top[top_ctr++] = i;
            }else{
                bottom[bottom_ctr++] = i;
            }
        }
        for (i=0; i < top_ctr; ++i){
            newState[i] = top[i];
        }
        for (i=0; i < bottom_ctr; ++i){
            newState[i+top_ctr] = bottom[i];
        }
        
        //state = newState;
        for (i=0; i < N; ++i){
            state[i] = newState[i];
            
        }
    }
    
    for (i=0; i < N; ++i){
        printf("%d ", state[i] );
    }
}


struct PAIR{
    int  first;
    int second;
    int marked;
};
typedef struct PAIR PARA;


PARA* makePairs(){
    
    int i,j, size = 0;
    
    //how to get index of pair?
    size = N * (N-1) / 2;
    
    PARA *pairs = (PARA*)malloc(sizeof(PARA) * size);
    
    int k = 0;
    for(i=0; i <= N-2; i++){
        for(j=i+1; j <= N-1; j++){
            pairs[k].first = i;
            pairs[k].second = j;
            pairs[k].marked = 0;
            k++;
            //printf("pair (%d, %d) created\n", pairs[k].first, pairs[k].second);
        }
    }
    
    return pairs;
}
int markedPairs = 0;
void markPairs(int *top, int topSize, PARA *pairs){
    
    int i,j, size = N * (N-1) / 2;
    int counter;
    for(i=0; i < size; i++){
        if (!pairs[i].marked){
            counter = 0;
            for (j = 0; j < topSize; j++) {
                if (pairs[i].first == top[j] ||
                    pairs[i].second == top[j]){
                    counter++;
                }
            }
            if (counter == 1) {
                pairs[i].marked = 1;
                markedPairs++;
                
                int f = pairs[i].first;
                int s = pairs[i].second;
                //printf("pair (%d, %d) marked\n", f, s);
                //printf("%d remain \n", size - markedPairs);
            }
        }
    }
}

void ksa_rs_sst(unsigned char state[], unsigned char key[])
{
    int i, r = 0;
    int top[256], bottom[256];
    int top_ctr = 0, bottom_ctr = 0;
    int len = strlen((char*)key);
    int size = N * (N-1) / 2;
    
    unsigned char newState[256];
    
    for (i=0; i < N; ++i)
        state[i] = i;
    
    PARA *pairs = makePairs();
    
    while (markedPairs < size){
        
        
        top_ctr = 0;
        bottom_ctr = 0;
        
        for (i=0; i < N; ++i){
            if (bitOfKey(key, (r*N + i) % (len*8)) == 0){
                top[top_ctr++] = i;
            }else{
                bottom[bottom_ctr++] = i;
            }
        }
        
        for (i=0; i < top_ctr; ++i){
            newState[i] = top[i];
        }
        for (i=0; i < bottom_ctr; ++i){
            newState[i+top_ctr] = bottom[i];
        }
        
        //check pairs
        markPairs(top, top_ctr, pairs);
        //state = newState;
        for (i=0; i < N; ++i){
            state[i] = newState[i];
            
        }
        r++;
    }
    printf("r = %d\n", r );
    for (i=0; i < N; ++i){
        printf("%d ", state[i] );
    }
}



// Pseudo-Random Generator Algorithm
void prga(unsigned char state[], int len)
{
    int i=0,j=0,x,t;
    unsigned char result = 0;
    
    char stateFor64 = 0;
    
    for (x=0; x < len; ++x)  {
        i = (i + 1) % N;
        j = (j + state[i]) % N;
        t = state[i];
        state[i] = state[j];
        state[j] = t;
        
        if(N == 256){ // 8 bits generated
            result = state[(state[i] + state[j]) % N];
            fwrite(&result, sizeof(char), 1, file);
        }else if (N == 64){  // 6 bits generated
            
            if(stateFor64 == 0){
                result = state[(state[i] + state[j]) % N];
                result <<= 2;
            }else if (stateFor64 == 1){
                unsigned char newState = state[(state[i] + state[j]) % N];
                unsigned char tailOfPrevState = newState >> 4;
                result += tailOfPrevState;
                fwrite(&result, sizeof(char), 1, file);
                
                result = newState << 4;
            } else if (stateFor64 == 2){
                unsigned char newState = state[(state[i] + state[j]) % N];
                result += newState >> 2;
                fwrite(&result, sizeof(char), 1, file);
                
                result = newState << 6;
            }else{ // 4 bits generated
                unsigned char newState = state[(state[i] + state[j]) % N];
                result += newState;
                fwrite(&result, sizeof(char), 1, file);
            }
            
            stateFor64 = (stateFor64 + 1) % 4;
        }else{
            if (x % 2 == 0){
                result = state[(state[i] + state[j]) % N];
            }else{
                result <<= 4;
                result += state[(state[i] + state[j]) % N];
                fwrite(&result, sizeof(char), 1, file);
            }
        }
        
        
    }
}

int main()
{
    unsigned char state[256],key[]={"рпропорптоп5th74t8grнек5увн45вм445и68798тн87пь078"};
    int len=16000000;
    puts("test");
    file = fopen("rc4_sst_64.bin", "wb");
    
    
    ksa_rs_sst(state,key);
    prga(state,len);
    
    fclose(file);
    
    return 0;
}
