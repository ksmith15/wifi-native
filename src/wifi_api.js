// Copyright (c) 2014 Jaguar Land Rover. All rights reserved.
//

extension.setMessageListener( function( json ) {
    var msg = JSON.parse( json );
    if ( msg.cmd == 'SetPowered' )
        extension.HandleSetPowered( msg );
    else if ( msg.cmd == 'GetPowered' )
        extension.HandleGetPowered( msg );
    else if ( msg.cmd == 'SetTethering' )
        extension.HandleGetPowered( msg );
    else if ( msg.cmd == 'Connect' )
        extension.HandleGetPowered( msg );
    else if ( msg.cmd == 'Disconnect' )
        extension.HandleGetPowered( msg );
    else if ( msg.cmd == 'SetAutoConnect' )
        extension.HandleGetPowered( msg );
    else if ( msg.cmd == 'GetTechnologies' )
        extension.HandleGetPowered( msg );
    else if ( msg.cmd == 'GetServices' )
        extension.HandleGetPowered( msg );
    else if ( msg.cmd == 'Scan' )
        extension.HandleGetPowered( msg );
});

