/**
 * Author: Jake Wright (github.com/wjake)
 * Date: 2025-02-24
 * File: UDPReceiver.h
 * Class: UUDPReceiver
 * Description: Actor component for receiving and processing UDP messages with event-driven functionality.
 * Dependencies: Networking.h, Sockets.h, UdpSocketReceiver.h
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Common/UdpSocketReceiver.h"
#include "Common/UdpSocketBuilder.h"
#include "Sockets.h"
#include "Async/Async.h"
#include "UUDPReceiver.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogUDPReceiver, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMessageReceivedEvent,
    UUDPReceiver*, ReceiverInstance,  const FString&, SenderIP, const FString&, ReceivedMessage);

UCLASS(ClassGroup = (Server), meta = (BlueprintSpawnableComponent))
class UUDPReceiver : public UActorComponent
{
    GENERATED_BODY()

public:
    UUDPReceiver();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP Config")
    FString SocketName = TEXT("Socket");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP Config")
    FString IP = TEXT("0.0.0.0");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP Config")
    int32 Port = 65432;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP Config")
    int32 BufferSize = 1024;

    /**
     * @brief Start the UDP receiver.
     * @return The UDP receiver instance.
     */
    UFUNCTION(BlueprintCallable, Category = "UDP Receiver")
    UUDPReceiver* StartReceiver();

    /**
     * @brief Stop the UDP receiver.
     * @return The UDP receiver instance.
     */
    UFUNCTION(BlueprintCallable, Category = "UDP Receiver")
    UUDPReceiver* StopReceiver();

    /**
     * @brief Update the UDP receiver configuration.
     * @param NewIP The new IP address.
     * @param NewPort The new port number.
     * @param NewBufferSize The new buffer size.
     * @return The UDP receiver instance.
     */
    UFUNCTION(BlueprintCallable, Category = "UDP Config")
    UUDPReceiver* UpdateConfig(const FString& NewIP, int32 NewPort, int32 NewBufferSize);

    /**
     * @brief Send a message to a specified recipient.
     * @param Message The message to send.
     * @param ReceipientIP The IP address of the recipient.
     * @return True if the message was sent successfully, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "UDP Receiver")
    bool SendMessage(const FString& Message, const FString& ReceipientIP);

    /**
     * @brief Event for handling received messages.
     */
    UPROPERTY(BlueprintAssignable, Category = "UDP Receiver")
    FMessageReceivedEvent OnMessageReceivedEvent;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
 
private:
    FSocket* UdpSocket;
    TSharedPtr<FUdpSocketReceiver> UdpSocketReceiver;

    /**
     * @brief Callback function for handling received UDP messages.
     * @param Data The received data.
     * @param Endpoint The endpoint from which the data was received.
     */
    void OnMessageReceived(const TSharedPtr<FArrayReader, ESPMode::ThreadSafe>& Data, const FIPv4Endpoint& Endpoint);
};