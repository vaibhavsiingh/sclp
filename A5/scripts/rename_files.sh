for f in testcases/L4-valid/*.tac; do
    mv "$f" "${f%.tac}.old.tac"
done