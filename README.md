# Connect4 <img src="https://github.com/RichardS0268/Connect4/blob/main/connect4.png" height=50px; />

## Description

Using MCTS and UCT algorithms play Connect 4 (The rules are a little bit different. [More details](https://docs.saiblo.net/games/connect4/connect4.html)). Evaluation is based on competition results with existing `.so` in `./TestCases`.

## Inference

```sh
bash compile.sh
bash compete.sh 5 // how many rounds with each .so you want to play
python stat.py
```

Output example<img src="https://github.com/RichardS0268/Connect4/blob/main/output_example.png" alt="image-20220608235736404" style="zoom:30%;" />


