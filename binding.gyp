{
  "targets": [
    {
      "target_name": "node-ems",
      "sources": [ "node-ems.cc" ],
      "include_dirs": ["/Users/jens/tibco/ems/8.0/include"],
      "libraries": ["-ltibems64","-ltibemslookup64","-ltibemsufo64","-ltibemsadmin64","-lldap","-llber","-lxml2","-lssl","-lcrypto","-lz","-lpthread","-L/Users/jens/tibco/ems/8.0/lib"]
    }
  ]
}
