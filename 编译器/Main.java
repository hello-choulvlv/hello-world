/*
  *@aim:主函数
  *@date:2015-10-07 19:14:13
  */
  import    lexer.Lexer;
  import    inter.Parser;
  import    tool.FileManager;
  import    java.io.IOException;
  import    java.io.File;
  import    java.io.FileInputStream;
  import    java.io.PrintStream;
  public    class    Main
 {
	public    static    void    main(String[]    argv)throws   IOException
	{
		//                              System.out.println(argv.length);
		//                              System.exit(0);
		  if(argv.length<2)
		{
					  System.out.println("Error: usage   source file,  output file.");
					  System.exit(1);
		 }
		  File    file=new    File(argv[0]);
		  if( !file.exists())
		 {
					  System.out.println("Error: source file does not exist!");
					  System.exit(2);
		  }
	//如果表示一个目录
		  if( ! file.isFile()  )
		 {
					  System.out.println("Error:source file is not a normal file!");
					  System.exit(3);
		  }
		  File         output=new   File(argv[1]);
		  if( output.exists() && !output.isFile())
		 {
						 System.out.println("Error: output file exists,and it is not a normal file.");
						 System.exit(4);
		  }
		  if( output.exists() )
		 {
						 if( !output.delete() )
						{
									   System.out.println("Error:delete  original  output file failure.");
									   System.exit(5);
						 }
		  }
		  if(  !output.createNewFile() )
		 {
						 System.out.println("Error:create output file failure.");
						 System.exit(6);
		  }
		  FileInputStream    input=new   FileInputStream(file);
		  FileManager.initWith(input,new   PrintStream(output));
		  LuaLexer          lex=new    LuaLexer();
		  LuaParser         parser=new    LuaParser(lex);
		  parser.program();
		  FileManager.input.close();
		  FileManager.output.close();
		  FileManager.input=null;
		  FileManager.output=null;
	}
  }