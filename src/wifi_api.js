// Copyright (c) 2015 Jaguar Land Rover. All rights reserved.
//

extension.setMessageListener( function( json ) {
    var msg = JSON.parse( json );
    if ( msg.cmd == 'SetPowered' )
        handleSetPowered( msg );
    else if ( msg.cmd == 'GetPowered' )
        handleGetPowered( msg );
    else if ( msg.cmd == 'SetTethering' )
        handleSetTethering( msg );
    else if ( msg.cmd == 'Connect' )
        handleConnect( msg );
    else if ( msg.cmd == 'Disconnect' )
        handleDisconnect( msg );
    else if ( msg.cmd == 'SetAutoConnect' )
        handleSetAutoConnect( msg );
    else if ( msg.cmd == 'GetTechnologies' )
        handleGetTechnologies( msg );
    else if ( msg.cmd == 'GetServices' )
        handleGetServices( msg );
    else if ( msg.cmd == 'Scan' )
        handleScan( msg );
});

exports.handleSetPowered = function( msg ) {
    console.log( 'handleSetPowered' );
}

exports.handleGetPowered = function( msg ) {
    console.log( 'handleGetPowered' );
}

exports.handleSetTethering = function( msg ) {
    console.log( 'handleSetTethering' );
}

exports.handleConnect = function( msg ) {
    console.log( 'handleConnect' );
}

exports.handleDisconnect = function( msg ) {
    console.log( 'handleDisconnect' );
}

exports.handleSetAutoConnect = function( msg ) {
    console.log( 'handleSetAutoConnect' );
}

exports.handleGetTechnologies = function( msg ) {
    console.log( 'handleGetTechnologies' );
}

exports.handleGetServices = function( msg ) {
    console.log( 'handleGetServices' );
}

exports.handleScan = function( msg ) {
    console.log( 'handleScan' );
}

