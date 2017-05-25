var objs = new Array();

function readFile(err,data)
{
  if(err)
  {
    return console.log(err);
  }
  objs.push(JSON.parse(data));
}

fs = require('fs')
var count = process.argv.length;
while(count-- > 2)
{
    fs.readFile(process.argv[count], 'utf8', readFile);
}
go();

function go()
{
  if(objs.length < process.argv.length - 2)
    setTimeout(go, 200);
  else
  {
    console.log("void sequence_"+Math.random().toString(36).substring(2)+"(int32_t x, int32_t y)");
    console.log("{");
    console.log("    int32_t status = 0;");
    var j = 0;
    var stageOnlyMix = false;
    while(objs[j] != null)
    {
      if(objs[j].stage_only)
        stageOnlyMix = true;
      j++;
    }
    j = 0;
    while(objs[0] != null && objs[0]["sprite"+j] != null)
    {
      console.log("    // BEGIN_FRAME");
      for(var i = 0; i < objs.length; i++)
      {
        if(objs[i].stage_only)
        {
          console.log("    boxer::setStage("+objs[i].stage+j+");");
        }
        else
        {
          if(!stageOnlyMix)
          {
            console.log("    boxer::setStage("+objs[i].stage+");");
          }
          console.log("    status = boxer::blockResource("+objs[i]["sprite"+j]+", x, y);");
          console.log("    assert(status == 0);");
        }
      }
      console.log("    boxer::showStage();");
      console.log("    // END");
      j++;
    }
    console.log("}");
  }
}
