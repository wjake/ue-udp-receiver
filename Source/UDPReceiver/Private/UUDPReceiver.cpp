/**
 * Author: Jake Wright (github.com/wjake)
 * Date: 2025-02-24
 * File: UDPReceiver.cpp
 * Class: UUDPReceiver
 * Description: Actor component for receiving and processing UDP messages with event-driven functionality.
 * Dependencies: Networking.h, Sockets.h, UdpSocketReceiver.h
 */

#include "UUDPReceiver.h"

UUDPReceiver::UUDPReceiver()
{
    bAutoActivate = true;
}

void UUDPReceiver::BeginPlay()
{
    Super::BeginPlay();
    if (bAutoActivate)
    {
        StartReceiver();
    }
}

void UUDPReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    StopReceiver();
}

UUDPReceiver* UUDPReceiver::StartReceiver()
{
    FIPv4Address Addr;
    if (!FIPv4Address::Parse(IP, Addr))
    {
        UE_LOG(LogUDPReceiver, Error, TEXT("Invalid IP Address: %s"), *IP);
        return nullptr;
    }

    FIPv4Address::Parse(IP, Addr);
    FIPv4Endpoint Endpoint(Addr, Port);

    // Create and configure the UDP socket
    UdpSocket = FUdpSocketBuilder(SocketName)
        .AsNonBlocking()
        .AsReusable()
        .BoundToEndpoint(Endpoint)
        .WithReceiveBufferSize(BufferSize)
        .Build();
    
    if (!UdpSocket)
    {
        UE_LOG(LogUDPReceiver, Error, TEXT("Failed to create UDP socket with Name: %s"), *SocketName);
        return nullptr;
    }

    // Initialise the UDP socket receiver
    UdpSocketReceiver = MakeShareable(new FUdpSocketReceiver(UdpSocket, FTimespan::FromMilliseconds(100), TEXT("UdpSocketReceiver")));
    UdpSocketReceiver->OnDataReceived().BindUObject(this, &UUDPReceiver::OnMessageReceived);
    UdpSocketReceiver->Start();
    UE_LOG(LogUDPReceiver, Warning, TEXT("UDP receiver started."));

    return this;
}

UUDPReceiver* UUDPReceiver::StopReceiver()
{
    if (UdpSocketReceiver.IsValid())
    {
        UdpSocketReceiver->Stop();
        UdpSocketReceiver = nullptr;
    }
    if (UdpSocket)
    {
        UdpSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(UdpSocket);
        UdpSocket = nullptr;
    }

    return this;
}

UUDPReceiver* UUDPReceiver::UpdateConfig(const FString& NewIP, int32 NewPort, int32 NewBufferSize)
{
    IP = NewIP;
    Port = NewPort;
    BufferSize = NewBufferSize;

    UE_LOG(LogUDPReceiver, Log, TEXT("Updated Config - IP: %s, Port: %d, BufferSize: %d"), *IP, Port, BufferSize);

    return this;
}


bool UUDPReceiver::SendMessage(const FString& Message, const FString& ReceipientIP)
{
    bool bMessageSent = false;

    FIPv4Address Addr;
    if (!FIPv4Address::Parse(ReceipientIP, Addr))
    {
        UE_LOG(LogUDPReceiver, Error, TEXT("Invalid IP Address: %s"), *ReceipientIP);
        return bMessageSent;
    }

    FIPv4Endpoint Endpoint(Addr, Port);
    TArray<uint8> DataBytes;
    FTCHARToUTF8 Convert(*Message);
    DataBytes.Append(reinterpret_cast<const uint8*>(Convert.Get()), Convert.Length());

    int32 BytesSent = 0;
    if (UdpSocket->SendTo(DataBytes.GetData(), DataBytes.Num(), BytesSent, *Endpoint.ToInternetAddr()))
    {
        UE_LOG(LogUDPReceiver, Log, TEXT("Sent reply: '%s' to %s"), *Message, *Endpoint.ToString());
        return bMessageSent = true;
    }
    else
    {
        UE_LOG(LogUDPReceiver, Error, TEXT("Failed to send reply to %s"), *Endpoint.ToString());
        return bMessageSent;
    }

    return bMessageSent;
}

void UUDPReceiver::OnMessageReceived(const TSharedPtr<FArrayReader, ESPMode::ThreadSafe>& Data, const FIPv4Endpoint& Endpoint)
{
    // Null-terminate the received data
    TArray<uint8> ReceivedDataArray = *Data;
    ReceivedDataArray.Add(0);

    // Handle received gesture data
    FString ReceivedDataString = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedDataArray.GetData())));
    FString SenderIP = Endpoint.Address.ToString();

    UE_LOG(LogUDPReceiver, Warning, TEXT("Received data: %s from %s"), *ReceivedDataString, *Endpoint.ToString());

    // Broadcast received data to delegate
    AsyncTask(ENamedThreads::GameThread, [this, ReceivedDataString, SenderIP]()
    {
        OnMessageReceivedEvent.Broadcast(this, SenderIP, ReceivedDataString);
    });

    // Prepare response
    FString ResponseData = TEXT("Data received & processed");
    TArray<uint8> ResponseBytes;
    FTCHARToUTF8 Convert(*ResponseData);
    ResponseBytes.Append((uint8*)Convert.Get(), Convert.Length());

    // Send the response to the mystic client
    int32 BytesSent;
    UdpSocket->SendTo(ResponseBytes.GetData(), ResponseBytes.Num(), BytesSent, *Endpoint.ToInternetAddr());
    UE_LOG(LogUDPReceiver, Warning, TEXT("Sent data: %s to %s"), *ResponseData, *Endpoint.ToString());
}
