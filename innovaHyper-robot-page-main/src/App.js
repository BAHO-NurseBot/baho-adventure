import React, { Component } from "react";
import "./App.css";
import axios from "axios";
import Pusher from "pusher-js";
import heatbeat from "./img/heartBeat_img.png";
import weightImg from "./img/weight.png";
import heightImg from "./img/height.png";
import temperatureImg from "./img/temperature.psd.png";
import oxygenImg from "./img/oxygen.png";
import { name } from "./constant";

const baseURL = "https://innovahyperbackend.onrender.com";

class App extends Component {
  constructor(props) {
    super(props);
    this.state = {
      records: null,
      activeCard: false,
      activeCardInfo: null,
    };
  }
  componentDidMount() {
    const pusher = new Pusher("8ad9cdb032d448d5bc06", {
      cluster: "ap2",
      encrypted: true,
    });

    const cardChannel = pusher.subscribe("robot");
    cardChannel.bind("card", (data) => {
      console.log(data);
      if (data.isActive) {
        this.fetchActiveCardData();
      }
      this.setState({ activeCard: data.isActive });
    });

    axios.get(baseURL + "/medicalRecords/active-card").then((response) => {
      console.log(response.data.message);
      if (response.data.message !== "No user with active card") {
        this.setState({
          activeCard: true,
        });
        this.fetchActiveCardData();
      } else {
        this.setState({
          activeCard: false,
        });
      }
    });

    const channel = pusher.subscribe("robot");
    channel.bind("medical_records_data", (data) => {
      this.setState({ records: data.data });
    });
  }

  fetchActiveCardData() {
    axios.get(baseURL + "/medicalRecords/medical-records").then((response) => {
      console.log(response.data.error);
      if (response.data.error === "There is no active card!") {
        this.activeCard = false;
        this.setState({
          records: null,
        });
      } else {
        console.log("In the fetch active data");
        console.log(response.data);
        this.setState({
          records: response.data.data,
        });
      }
    });
  }

  // render() {
  //   const { records, activeCard } = this.state;
  //   return (
  //     <div className="App">
  //       <header className="App-header">
  //         <div className="bg-transparent">
  //           <div className="card">
  //             <h4>Welcome to {name}!</h4>
  //             {activeCard ? (
  //               <>
  //                 {records ? (
  //                   <>
  //                     <div className="card-id">
  //                       Card Number: {records.cardId}
  //                     </div>
  //                     <h5>Names: {records.fullName}</h5>

  //                     <div className="flex left-top">
  //                       <img src={temperatureImg} alt="heat" className="icon" />
  //                       <div>
  //                         <div className="text-sm">Temperature:</div>
  //                         <div>{records.temperature} °C</div>
  //                       </div>
  //                     </div>
  //                     <div className="flex right-top">
  //                       <div>
  //                         <div className="text-sm">Height:</div>
  //                         <div>{records.height} Cm</div>
  //                       </div>
  //                       <img src={heightImg} alt="heat" className="icon" />
  //                     </div>
  //                     <div className="flex left-bottom">
  //                       <img src={weightImg} alt="heat" className="icon" />
  //                       <div>
  //                         <div className="text-sm">Weight:</div>
  //                         <div>{records.weight} Kg</div>
  //                       </div>
  //                     </div>

  //                     <div className="flex right-bottom">
  //                       <div>
  //                         <div className="text-sm">Blood pressure:</div>
  //                         <div>{records.bloodPressure} bpm</div>
  //                       </div>
  //                       <img src={heatbeat} alt="heat" className="icon" />
  //                     </div>

  //                     <div className="center-bottom">
  //                       <div>
  //                         <div className="text-sm">Oxygen Level:</div>
  //                         <div>{records.oxygen}%</div>
  //                       </div>
  //                       <img src={oxygenImg} alt="heat" className="icon" />
  //                     </div>
  //                   </>
  //                 ) : (
  //                   <p>Loading...</p>
  //                 )}
  //               </>
  //             ) : (
  //               <>
  //                 <p className="tap">TAP TO CONTINUE!</p>
  //                 <p className="noactivecard">No active card!</p>
  //               </>
  //             )}
  //           </div>
  //         </div>
  //       </header>
  //     </div>
  //   );
  // }

  render() {
    const { records, activeCard } = this.state;
    return (
      <div className="App">
        <header className="App-header">
          <div className="bg-transparent">
            <div className="card">
              <h4>Welcome to {name}!</h4>
              {activeCard ? (
                <>
                  {records ? (
                    <>
                      <div className="card-id">
                        Card Number: {records.cardId}
                      </div>
                      <h5>Names: {records.fullName}</h5>
                      <div className="flex left-top">
                        <img src={temperatureImg} alt="heat" className="icon" />
                        <div>
                          <div className="text-sm">Temperature:</div>
                          <div>{records.temperature} °C</div>
                        </div>
                      </div>
                      <div className="flex right-top">
                        <div>
                          <div className="text-sm">Height:</div>
                          <div>{records.height} Cm</div>
                        </div>
                        <img src={heightImg} alt="height" className="icon" />
                      </div>
                      <div className="flex left-middle">
                        <img src={weightImg} alt="weight" className="icon" />
                        <div>
                          <div className="text-sm">Weight:</div>
                          <div>{records.weight} Kg</div>
                        </div>
                      </div>

                      <div className="flex right-middle">
                        <div>
                          <div className="text-sm">Pulse Rate:</div>
                          <div>{records.bloodPressure} bpm</div>
                        </div>
                        <img src={heatbeat} alt="heartbeat" className="icon" />
                      </div>

                      <div className="center-bottom">
                        <div>
                          <div className="text-sm">Oxygen Level:</div>
                          <div>{records.oxygen}%</div>
                        </div>
                        <img src={oxygenImg} alt="oxygen" className="icon" />
                      </div>
                    </>
                  ) : (
                    <p>Loading...</p>
                  )}
                </>
              ) : (
                <>
                  <p className="tap">TAP TO CONTINUE!</p>
                  <p className="noactivecard">No active card!</p>
                </>
              )}
            </div>
          </div>
        </header>
      </div>
    );
  }
}

export default App;
