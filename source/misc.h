int is2pow(int num){
    int index=1;
    int i;
    for(i=0; i<32; i++)
        if(num == index)
            return 1;
        else
            index <<= 1;
    return 0;
}
