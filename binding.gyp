{
  "targets": [
    {
      "target_name": "node-ems",
      "sources": [ "node-ems.cc" , "emsconnection.cc"],
      "include_dirs": ["$(EMS_HOME)/include"],
      "libraries": ["-ltibems64","-ltibemslookup64","-ltibemsufo64","-ltibemsadmin64","-lldap","-llber","-lxml2","-lssl","-lcrypto","-lz","-lpthread","-L$(EMS_HOME)/lib"]
    }
  ]
}
