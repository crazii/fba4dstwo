
for (int i=0; i<0x10000; i++) 
{
    	if(((unsigned int)sh2JumpTable[i])>>16==0xFEDC)
    	{
    		sh2JumpTable[i]=sh2JumpTable[((unsigned int)sh2JumpTable[i])&0xFFFF];
    	}	
}
